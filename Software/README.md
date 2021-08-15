## 编译说明
* MCU固件: 务必使用**Keil v5.25**或以上的版本进行编译（因为旧编译器不能完全支持**C++ 11**的语法）,并安装[雅特力](https://www.arterytek.com/cn/index.jsp)官方PACK->[AT32F4xx标准外设库](http://www.arterytek.com/download/Pack_Keil_AT32F4xx_CH_V1.3.4.zip)。
* VS模拟器: 使用**Visual Studio 2019**编译，配置为**Release x86**。在`Config.h`里修改`CONFIG_TRACK_VIRTUAL_GPX_FILE_PATH`宏定义指定被读取的GPX文件的路径。

## 系统配置文件
系统会在根目录下自动生成`SystemSave.json`的文件，用于储存和配置系统参数:
```C
{
  "sportStatus.totalDistance": 0,              //总里程(m)
  "sportStatus.totalTimeUINT32[0]": 0,         //总行驶时间(ms)，低32位
  "sportStatus.totalTimeUINT32[1]": 0,         //总行驶时间(ms)，高32位
  "sportStatus.speedMaxKph": 0,                //最高时速(km/h)
  "sportStatus.weight": 65,                    //体重(kg)
  "GMT_OffsetHours": 8,                        //时区(GMT+)
  "sysConfig.soundEnable": 1,                  //系统提示音使能(1:开启，0:关闭)
  "sysConfig.longitudeDefault": 116.391332,    //默认位置(经度)
  "sysConfig.latitudeDefault": 39.907415,      //默认位置(纬度)
  "sysConfig.language": "en-GB",               //语言(尚不支持多语言切换)
  "sysConfig.mapDirPath": "/Bing",             //存放地图的文件夹路径
  "sysConfig.WGS84": 1,                        //坐标系统配置(0:GCJ02, 1:WGS84)
  "sysConfig.arrowTheme": "default"            //导航箭头主题(default:全黑，dark:橙底黑边，light:橙底白边)
}
```
