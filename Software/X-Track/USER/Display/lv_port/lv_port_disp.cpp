#include "../Display.h"

static lv_disp_drv_t* disp_drv_p;

#define DISP_HOR_RES         CONFIG_SCREEN_HOR_RES
#define DISP_VER_RES         CONFIG_SCREEN_VER_RES
#define DISP_BUF_SIZE        CONFIG_SCREEN_BUFFER_SIZE
#define DISP_USE_DMA         1
#define DISP_DMA_CHANNEL     DMA1_Channel3
#define DISP_DMA_MAX_SIZE    65535

static lv_color_t lv_full_disp_buf[DISP_BUF_SIZE];
static lv_color_t* lv_disp_buf1 = lv_full_disp_buf;

static uint8_t* disp_dma_tar_p = NULL;
static uint8_t* disp_dma_cur_p = NULL;

/**
  * @brief  ʹ��DMA���ͻ���������
  * @param  buf:��������ַ
  * @param  size:����������
  * @retval ��
  */
static void disp_spi_dma_send(const void* buf, uint32_t size)
{
    if(size > DISP_DMA_MAX_SIZE)
    {
        if(disp_dma_tar_p == NULL)
        {
            disp_dma_tar_p = (uint8_t*)buf + size;
        }
        disp_dma_cur_p = (uint8_t*)buf + DISP_DMA_MAX_SIZE;
        size = DISP_DMA_MAX_SIZE;
    }
    else
    {
        disp_dma_cur_p = NULL;
        disp_dma_tar_p = NULL;
    }
    
    DMA_ChannelEnable(DISP_DMA_CHANNEL, DISABLE);
    DISP_DMA_CHANNEL->CMBA = (uint32_t)buf;
    DISP_DMA_CHANNEL->TCNT = size;
    DMA_ChannelEnable(DISP_DMA_CHANNEL, ENABLE);
}

/**
  * @brief  ��Ļˢ�»ص�����
  * @param  disp:��Ļ������ַ
  * @param  area:ˢ������
  * @param  color_p:ˢ�»�������ַ
  * @retval ��
  */
static void disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    SCREEN_CLASS* screen = (SCREEN_CLASS*)disp->user_data;
    
    disp_drv_p = disp;

#if DISP_USE_DMA
    const lv_coord_t w = (area->x2 - area->x1 + 1);
    const lv_coord_t h = (area->y2 - area->y1 + 1);
    const uint32_t size = w * h * sizeof(lv_color_t);

    /*����ˢ������*/
    screen->setAddrWindow(area->x1, area->y1, area->x2, area->y2);

    /*����ģʽ*/
    digitalWrite_LOW(CONFIG_SCREEN_CS_PIN);
    digitalWrite_HIGH(CONFIG_SCREEN_DC_PIN);

    /*DMA��������*/
    disp_spi_dma_send(color_p, size);
#else
    screen->drawFastRGBBitmap(area->x1, area->y1, (uint16_t*)color_p, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
    lv_disp_flush_ready(disp_drv_p);
#endif
}

extern "C" {
    void DMA1_Channel3_IRQHandler(void)
    {
        /*DMA��������ж�*/
        if(DMA_GetITStatus(DMA1_INT_TC3) != RESET)
        {
            DMA_ClearITPendingBit(DMA1_INT_TC3);
            if(disp_dma_cur_p < disp_dma_tar_p)
            {
                disp_spi_dma_send(disp_dma_cur_p, disp_dma_tar_p - disp_dma_cur_p);
            }
            else
            {
                digitalWrite_HIGH(CONFIG_SCREEN_CS_PIN);
                lv_disp_flush_ready(disp_drv_p);
            }
        }
    }
}

/**
  * @brief  DMA��ʼ��
  * @param  ��
  * @retval ��
  */
static void disp_spi_dma_init()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA1, ENABLE);

    DMA_Reset(DISP_DMA_CHANNEL);

    DMA_InitType DMA_InitStructure;
    DMA_DefaultInitParaConfig(&DMA_InitStructure);
    
    DMA_InitStructure.DMA_Mode = DMA_MODE_NORMAL;  //��������������ģʽ
    DMA_InitStructure.DMA_MTOM = DMA_MEMTOMEM_DISABLE;  //DMAͨ�� û������Ϊ�ڴ浽�ڴ洫��
    DMA_InitStructure.DMA_Direction = DMA_DIR_PERIPHERALDST;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
    DMA_InitStructure.DMA_Priority = DMA_PRIORITY_MEDIUM; //DMAͨ�� �����ȼ�
    
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)lv_disp_buf1;  //DMA�ڴ����ַ
    DMA_InitStructure.DMA_MemoryInc = DMA_MEMORYINC_ENABLE;  //�ڴ��ַ�Ĵ�������
    DMA_InitStructure.DMA_MemoryDataWidth = DMA_MEMORYDATAWIDTH_BYTE; //���ݿ��
    DMA_InitStructure.DMA_BufferSize = DISP_DMA_MAX_SIZE;  //DMAͨ����DMA����Ĵ�С
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI1->DT));  //DMA����SPI����ַ
    DMA_InitStructure.DMA_PeripheralInc = DMA_PERIPHERALINC_DISABLE;  //�����ַ�Ĵ�������
    DMA_InitStructure.DMA_PeripheralDataWidth = DMA_PERIPHERALDATAWIDTH_BYTE;  //���ݿ��

    DMA_Init(DISP_DMA_CHANNEL, &DMA_InitStructure);

    SPI_I2S_DMAEnable(SPI1, SPI_I2S_DMA_TX, ENABLE);

    NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    
    DMA_INTConfig(DISP_DMA_CHANNEL, DMA_INT_TC, ENABLE);
}

static void disp_wait_cb(lv_disp_drv_t* disp_drv)
{
    __wfi();
}

/**
  * @brief  ��Ļ��ʼ��
  * @param  ��
  * @retval ��
  */
void lv_port_disp_init(SCREEN_CLASS* scr)
{
    disp_spi_dma_init();

    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, lv_disp_buf1, NULL, DISP_BUF_SIZE);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.wait_cb = disp_wait_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = scr;
    lv_disp_drv_register(&disp_drv);
}
