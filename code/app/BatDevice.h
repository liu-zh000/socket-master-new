#ifndef _BAT_DEVICE_H
#define _BAT_DEVICE_H
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termio.h>
#include<stdlib.h>


#include <stdint.h> 

#include <sys/ioctl.h> 


#include <thread>
#include <chrono>
#include <memory>


class BatDevice{
public:
    BatDevice(){
            if((fd = open_port_(fd,3))<0)
            {    
            printf("open_port error\n");
          
            }
            if((i = set_opt_(fd,115200,8,'N',1))<0)
            {    
            printf("set_opt error\n");
            
            }
        threadPtr = std::make_unique<std::thread>(&BatDevice::work, this);
    }

    ~BatDevice(){
        stopThread();
        close(fd);
        
    }
    void stopThread() {
        // 标记停止线程的条件
        m_stopThread = true;
        // 等待线程结束
        if (threadPtr && threadPtr->joinable())
            threadPtr->join();
    }
    void work(){
 
        while(!m_stopThread)
        {
            do                                                                        
            {
                nread = read(fd,vcCmd_rece,128);     //接收
            }while(nread==0);
            //printf("Recv %d Bytes.\n",nread);
            //printf("电压值：%s\n",vcCmd_rece);
            V_revised = atof(vcCmd_rece);
            memset(vcCmd_rece,0x0,sizeof(vcCmd_rece));
            //printf("Waiting for 3 seconds.\n"); 	
            
            tcflush(fd, TCIFLUSH);
    
            sleep(1);
            
	    } 
	
    }
    float getVrevised(){
        return V_revised;
    }
private:
    int fd;
	int nread=0,i,j;
    char vcCmd_rece[128]; 
    float V_revised = 0.00;
    int set_opt_(int fd,int nSpeed,int nBits,char nEvent,int nStop)
	{     
	struct termios newtio,oldtio;
	
    /*
    struct termios
    {
        unsigned short c_iflag;  //输入模式
        unsigned short c_oflag;  //输出模式
        unsigned short c_cflag;  //控制模式
        unsigned short c_lflag;  //本地模式
        unsigned char c_cc[NCC]; //控制 字符 数据
    }
    */

	// 获取终端参数, 成功返回零；失败返回非零, 发生失败接口将设置errno错误标识 返回的结果保存在termios结构体
	if(tcgetattr(fd,&oldtio)!=0)
	{      //perror("SetupSerial 1");
		  return -1;
	}
	bzero(&newtio,sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;     // 控制模式标志,指定终端硬件控制信息
    
	/*设置数据位数*/
	switch(nBits)
	{     case 7 :
			  newtio.c_cflag |= CS7;
			  break;
		 case 8 :
			  newtio.c_cflag |= CS8;
			  break;
	}
	switch(nEvent)
	{     case 'N' :
			  newtio.c_cflag &= ~PARENB;
			  break;
		 case 'O' :
			  break;
	}

	/*设置波特率*/
	switch(nSpeed)
	{     case 2400 :
			  cfsetispeed(&newtio,B2400);
			  cfsetospeed(&newtio,B2400);
			  break;
		case 115200 :
			  cfsetispeed(&newtio,B115200);
			  cfsetospeed(&newtio,B115200);
			  break;
	   default :
			  cfsetispeed(&newtio,B9600);
			  cfsetospeed(&newtio,B9600);
			  break;
	}
	/*设置停止位*/
	if(nStop == 1)
	{      newtio.c_cflag &= ~CSTOPB;
	}
	else if(nStop == 2)
	{      newtio.c_cflag |= CSTOPB;
	}

	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;

	/* 
	清空终端未完成的输入/输出请求及数据
	TCIFLUSH 　清空输入缓存
	TCOFLUSH 　清空输入缓存
	TCIOFLUSH  清空输入输出缓存 
	*/
	// 清空终端未完成的输入/输出请求及数据
	tcflush(fd,TCIFLUSH); 

	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{     //perror("com set error");
		 return -1;
	}
	//printf("set done!\n");
	return 0;
}
    int open_port_(int fd,int comport)
	{    
	long vdisable;
	if(comport == 1)
	{     fd = open("/dev/ttySAC0",O_RDWR|O_NOCTTY|O_NDELAY);
		 if(fd == -1)
		 {     //perror("Can't Open Serial Port");
				return -1;
		 }
	}
	else if(comport == 2)
	{     fd = open("/dev/ttySAC1",O_RDWR|O_NOCTTY|O_NDELAY);
		 if(fd == -1)
		 {     
			 perror("Can't Open Serial Port");
				return -1;
		 }
	}
	else if(comport == 3)
	{
        //fd = open("/dev/ttyS2",O_RDWR|O_NOCTTY|O_NDELAY);
        //镭视融合核心板是通过/dev/ttyTHS1与电池电源管理单片机通讯。获取电量
        fd = open("/dev/ttyTHS1",O_RDWR|O_NOCTTY|O_NDELAY);
        if(fd == -1)
        {
            perror("Can't Open Serial Port");
            return -1;
        }
	}
	if(fcntl(fd,F_SETFL,0)<0)
	{     printf("fcntl failed!\n");
	}
	else
	{     printf("fcntl = %d\n",fcntl(fd,F_SETFL,0));
	}
	if(isatty(STDIN_FILENO)==0)
	{    printf("standard input is not a terminal device\n");
	}
	else
	{    printf("isatty success!\n");
	}
	printf("fd-open = %d\n",fd);
	return fd;
}
    std::unique_ptr<std::thread> threadPtr;
    bool m_stopThread = false;

};
//设置串口参数
// int BatDevice::set_opt_(int fd, int nSpeed, int nBits, char nEvent, int nStop)
// {
// 	struct termios newtio, oldtio;

// 	/*
// 	struct termios
// 	{
// 		unsigned short c_iflag;  //输入模式
// 		unsigned short c_oflag;  //输出模式
// 		unsigned short c_cflag;  //控制模式
// 		unsigned short c_lflag;  //本地模式
// 		unsigned char c_cc[NCC]; //控制 字符 数据
// 	}
// 	*/

// 	// 获取终端参数, 成功返回零；失败返回非零, 发生失败接口将设置errno错误标识 返回的结果保存在termios结构体
// 	if (tcgetattr(fd, &oldtio) != 0)
// 	{ // perror("SetupSerial 1");
// 		return -1;
// 	}
// 	bzero(&newtio, sizeof(newtio));
// 	newtio.c_cflag |= CLOCAL | CREAD;
// 	newtio.c_cflag &= ~CSIZE; // 控制模式标志,指定终端硬件控制信息

// 	/*设置数据位数*/
// 	switch (nBits)
// 	{
// 	case 7:
// 		newtio.c_cflag |= CS7;
// 		break;
// 	case 8:
// 		newtio.c_cflag |= CS8;
// 		break;
// 	}
// 	switch (nEvent)
// 	{
// 	case 'N':
// 		newtio.c_cflag &= ~PARENB;
// 		break;
// 	case 'O':
// 		break;
// 	}

// 	/*设置波特率*/
// 	switch (nSpeed)
// 	{
// 	case 2400:
// 		cfsetispeed(&newtio, B2400);
// 		cfsetospeed(&newtio, B2400);
// 		break;
// 	case 115200:
// 		cfsetispeed(&newtio, B115200);
// 		cfsetospeed(&newtio, B115200);
// 		break;
// 	default:
// 		cfsetispeed(&newtio, B9600);
// 		cfsetospeed(&newtio, B9600);
// 		break;
// 	}
// 	/*设置停止位*/
// 	if (nStop == 1)
// 	{
// 		newtio.c_cflag &= ~CSTOPB;
// 	}
// 	else if (nStop == 2)
// 	{
// 		newtio.c_cflag |= CSTOPB;
// 	}

// 	newtio.c_cc[VTIME] = 0;
// 	newtio.c_cc[VMIN] = 0;

// 	/*
// 	清空终端未完成的输入/输出请求及数据
// 	TCIFLUSH 　清空输入缓存
// 	TCOFLUSH 　清空输入缓存
// 	TCIOFLUSH  清空输入输出缓存
// 	*/
// 	// 清空终端未完成的输入/输出请求及数据
// 	tcflush(fd, TCIFLUSH);

// 	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
// 	{ // perror("com set error");
// 		return -1;
// 	}
// 	// printf("set done!\n");
// 	return 0;
// }
// int BatDevice::open_port_(int fd, int comport)
// {
// 	long vdisable;
// 	if (comport == 1)
// 	{
// 		fd = open("/dev/ttySAC0", O_RDWR | O_NOCTTY | O_NDELAY);
// 		if (fd == -1)
// 		{ // perror("Can't Open Serial Port");
// 			return -1;
// 		}
// 	}
// 	else if (comport == 2)
// 	{
// 		fd = open("/dev/ttySAC1", O_RDWR | O_NOCTTY | O_NDELAY);
// 		if (fd == -1)
// 		{
// 			perror("Can't Open Serial Port");
// 			return -1;
// 		}
// 	}
// 	else if (comport == 3)
// 	{
// 		// fd = open("/dev/ttyS2",O_RDWR|O_NOCTTY|O_NDELAY);
// 		// 镭视融合核心板是通过/dev/ttyTHS1与电池电源管理单片机通讯。获取电量
// 		fd = open("/dev/ttyTHS1", O_RDWR | O_NOCTTY | O_NDELAY);
// 		if (fd == -1)
// 		{
// 			perror("Can't Open Serial Port");
// 			return -1;
// 		}
// 	}
// 	if (fcntl(fd, F_SETFL, 0) < 0)
// 	{
// 		printf("fcntl failed!\n");
// 	}
// 	else
// 	{
// 		printf("fcntl = %d\n", fcntl(fd, F_SETFL, 0));
// 	}
// 	if (isatty(STDIN_FILENO) == 0)
// 	{
// 		printf("standard input is not a terminal device\n");
// 	}
// 	else
// 	{
// 		printf("isatty success!\n");
// 	}
// 	printf("fd-open = %d\n", fd);
// 	return fd;
// }
#endif

// #ifndef _BAT_DEVICE_H
// #define _BAT_DEVICE_H
// #include<stdio.h>
// #include<string.h>
// #include<sys/types.h>
// #include<errno.h>
// #include<sys/stat.h>
// #include<fcntl.h>
// #include<unistd.h>
// #include<termio.h>
// #include<stdlib.h>
// #define ADS1110 0x48   

// class BatDevice{
// public:
//     BatDevice(){
//         //打开文件
//         if((m_file=open("/dev/i2c-7", O_RDWR)) < 0) {    
//             perror("failed to open the bus\n");  
//         } 
//         //声明为IIC设备文件
//         if(ioctl(m_file, I2C_SLAVE, ADS1110) < 0) {    
//             perror("Failed to connect to the sensor\n"); 
//         }
//         threadPtr = std::make_unique<std::thread>(&BatDevice::work, this);
//     }

//     ~BatDevice(){
//         stopThread();
//         close(m_file);
        
//     }
//     void stopThread() {
//         // 标记停止线程的条件
//         m_stopThread = true;
//         // 等待线程结束
//         if (threadPtr && threadPtr->joinable())
//             threadPtr->join();
//     }
//     void work(){
//         //存储接收到的数据
//         unsigned char rev_data[3]={0}, raw_V0 = 0, raw_V1 = 0;   
//         float V_rev = 0, V_cal = 0.00, V_battery = 0.00;
        
//         int ini_times = 0;
//         while(!m_stopThread){
//             for(; ini_times < 3; ini_times++){      // 前三次的数据会不准确
//                 //先写，引起从机的反馈
//                 if(writeReg(m_file, ADS1110)==1){ 
//                     printf("error in writeReg ADS1110:%d\n", __LINE__);
//                 }           
//                 // 再读取从机反馈
//                 if(readReg(m_file, ADS1110, rev_data,3)==1){ 
//                     printf("error in readReg ADS1110");
//                 } 
//                 sleep(1);
//             }           
//             //先写，引起从机的反馈
//             if(writeReg(m_file, ADS1110)==1){ 
//                 printf("error in writeReg ADS1110:%d\n", __LINE__);
//             }           
//             // 再读取从机反馈
//             if(readReg(m_file, ADS1110, rev_data,3)==1){ 
//                 printf("error in readReg ADS1110:%d\n", __LINE__);
//             }           
//             raw_V0 = rev_data[0];
//             raw_V1 = rev_data[1];
//             V_rev = raw_V0 << 8 | raw_V1;           
//             V_cal = V_rev / 2048 * 2.048;
//             V_battery = V_cal * 7.5 + V_cal;
//             V_revised = V_battery + 0.08;         
// 		    // printf("V_revised: %.2f\n",V_revised);
            
//             sleep(1);
// 	    } 
	
//     }
//     float getVrevised(){
//         return V_revised;
//     }
// private:
//     int m_file; 
//     float V_revised = 0.00;


//     int writeReg(int pt, unsigned char add){   
//         unsigned char w_buff1[1], w_buff2[1]; 
//         //地址+写位，写位为0，所以只需要地址左移1位即可
//         w_buff1[0] = add << 1;      
//         //从机的读取协议
//         w_buff2[0] = 0x8c; 
//         //文件写操作——需要分两步来写，单步发送两次会出错
//         // 1.发送写指令
//         if(write(pt, w_buff1, 1)!=1) { 
//             perror("Failed to reset the read address\n"); 
//             return 1; 
//         }
//         //2.发送数据
//         if(write(pt, w_buff2, 1)!=1) {
//             perror("Failed to reset the read address\n"); 
//             return 1; 
//         }       
//         return 0; 
//     } 

//     int readReg(int pt, unsigned char s_add, unsigned char buf[], int size){ 
//         unsigned char writeBuff[1];

//             //读，从机地址+读位
//     	writeBuff[0]= (s_add<<1)|1; 

//     	//先把读的命令发给从机
//         if(write(pt, writeBuff, 1)!=1) { 
//             perror("Failed to reset the read address\n"); 
//             return 1; 
//         }       
//         //然后利用文件读操作，把读到的数据全部放到buf数组里面
//         if(read(pt, buf, size)!=size){ 
//             perror("Failed to read in the buffer\n"); 
//             return 1; 
//         }       
//         return 0; 
//     }
//     int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop);
//     int open_port(int fd,int comport);
//     std::unique_ptr<std::thread> threadPtr;
//     bool m_stopThread = false;

// };
// #endif
