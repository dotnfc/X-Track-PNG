/**
 * download map tiles from autonavi for X-Track
 * by dotnfc, 2021/08/16
 */

char *wget = "D:\\nfcshare\\bin\\wget.exe";
char *fmt = "http://webrd01.is.autonavi.com/appmaptile?x=%d&y=%d&z=%d&lang=zh_cn&size=1&scale=1&style=8";
char *sroot = "D:\\X-TRACK-1V6\\Map";
void download_z5()
{
    int z = 5;
    for ( int x=22; x<27; x++ ) 
    {
        for ( int y=10; y<15; y++ ) 
        {
            char url[1024];
            sprintf ( url, fmt, x, y, z );
            printf ( "%s\n", url );
            
            char out[256];
            sprintf ( out, "-O %d\\%d\\%d.png", z, x, y );
            printf ( "  => %s\n\n", out );
            
            char param[2048];
            sprintf ( param, "%s %s", url, out );
            exec ( wget, param, sroot, SW_HIDE );
        }
    }
}

void download_z6()
{
    int z = 6;
    char out[256];
    
    for ( int x=45; x<55; x++ ) 
    {
        sprintf ( out, "%s\\%d\\%d\\", sroot, z, x );
        mkdir ( out );

        for ( int y=20; y<28; y++ ) 
        {
            char url[1024];
            sprintf ( url, fmt, x, y, z );
            printf ( "%s\n", url );
            
            sprintf ( out, "-O %d\\%d\\%d.png", z, x, y );
            printf ( "  => %s\n\n", out );
                        
            char param[2048];
            sprintf ( param, "%s %s", url, out );
            exec ( wget, param, sroot, SW_HIDE );
        }
    }
}

void download_one(int z, int x, int y)
{
    char url[1024];
    char out[256];
    
    if ( y < 0 ) 
    {
        sprintf ( out, "%s\\%d\\%d\\", sroot, z, x );
        mkdir ( out );
        return;
    }
    
    sprintf ( url, fmt, x, y, z );
    printf ( "%s\n", url );
    
    sprintf ( out, "-O %d\\%d\\%d.png", z, x, y );
    printf ( "  => %s\n\n", out );
                
    char param[2048];
    sprintf ( param, "%s %s", url, out );
    exec ( wget, param, sroot, SW_HIDE );
}

void download_z7()
{
    download_one(7, 100, -1);
    download_one(7, 101, -1);
    download_one(7, 102, -1);
    download_one(7, 103, -1);
    download_one(7, 104, -1);
    download_one(7, 105, -1);
    download_one(7, 106, -1);
    
    download_one(7, 100, 52);    download_one(7, 100, 53);
    download_one(7, 101, 52);    download_one(7, 101, 53);
    download_one(7, 102, 52);    download_one(7, 102, 53);
    
    download_one(7, 103, 47);    download_one(7, 104, 47); download_one(7, 105, 47);    download_one(7, 106, 47);
    download_one(7, 103, 48);    download_one(7, 104, 48); download_one(7, 105, 48);    download_one(7, 106, 48);
    download_one(7, 103, 49);    download_one(7, 104, 49); download_one(7, 105, 49);    download_one(7, 106, 49);
}

void download_z8()
{
    download_one(8, 202, -1); download_one(8, 203, -1); download_one(8, 204, -1);
    
    download_one(8, 207, -1);download_one(8, 208, -1);
    download_one(8, 209, -1);
    download_one(8, 210, -1);
    download_one(8, 211, -1);
    
    download_one(8, 202, 105); download_one(8, 203, 105);
    download_one(8, 202, 106); download_one(8, 203, 106);
    
    download_one(8, 207, 95); download_one(8, 207, 96);
    download_one(8, 208, 95); download_one(8, 208, 96);
    
    download_one(8, 209, 95); download_one(8, 210, 95); download_one(8, 211, 95);
    download_one(8, 209, 96); download_one(8, 210, 96); download_one(8, 211, 96);
    download_one(8, 209, 97); download_one(8, 210, 97); download_one(8, 211, 97);
}

void download_z9()
{
}

void download_single()
{
    int x = 11;
    int y = 5;
    int z = 4;

    char buf[1024];
    sprintf ( buf, fmt, x, y, z );

    printf ( "%s\n", buf );
    clipboard_settext ( buf );
}

// download_z5();
//download_z6();
//download_z7();
download_z8();

//---------------------------------------------------------- end of script -----


