
#include "KernalUtil.h"
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/time.h>

std::vector<std::string> strSplit(const std::string s, const std::string& delim)
{
	std::vector<std::string> elems;
	size_t pos = 0;
	size_t len = s.length();
	size_t delim_len = delim.length();
	if(delim_len == 0) return elems;
	while(pos < len)
	{
		int find_pos = s.find(delim, pos);
		if(find_pos < 0)
		{
			elems.push_back(s.substr(pos, len - pos));
			break;
		}
		elems.push_back(s.substr(pos, find_pos - pos));
		pos = find_pos + delim_len;
	}
	return elems;
}

//获取地址
//返回IP地址字符串
//返回：0=成功，-1=失败
int getLocalIP(char* outip)
{
	int i=0;
	int sockfd;
	struct ifconf ifconf;
	char buf[512];
	struct ifreq *ifreq;
	char* ip;
	//初始化ifconf
	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
		return -1;
	}
	ioctl(sockfd, SIOCGIFCONF, &ifconf);//获取所有接口信息
	close(sockfd);
	//接下来一个一个的获取IP地址
	ifreq = (struct ifreq*)buf;

	for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
	{
		ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
		//排除127.0.0.1，继续下一个
		if(strcmp(ip,"127.0.0.1")==0)
		{
			ifreq++;
			continue;
		}
		strcpy(outip,ip);
		return 0;
	}

	return -1;
}

//获取地址
//返回MAC地址字符串
//返回：0=成功，-1=失败
int getLocalMac(char* mac)
{
	struct ifreq tmp;
	int sock_mac;
	char mac_addr[30];
	sock_mac = socket(AF_INET, SOCK_STREAM, 0);
	if( sock_mac == -1)
	{
		perror("create socket fail\n");
		return -1;
	}
	memset(&tmp,0,sizeof(tmp));
	strncpy(tmp.ifr_name,"eth0",sizeof(tmp.ifr_name)-1 );
	if( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 )
	{
		printf("mac ioctl error\n");
		return -1;
	}
	sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
			(unsigned char)tmp.ifr_hwaddr.sa_data[0],
			(unsigned char)tmp.ifr_hwaddr.sa_data[1],
			(unsigned char)tmp.ifr_hwaddr.sa_data[2],
			(unsigned char)tmp.ifr_hwaddr.sa_data[3],
			(unsigned char)tmp.ifr_hwaddr.sa_data[4],
			(unsigned char)tmp.ifr_hwaddr.sa_data[5]
			);
	close(sock_mac);
	memcpy(mac,mac_addr,strlen(mac_addr));
	return 0;
}

unsigned long int getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	//return tv.tv_sec; // 秒
	return tv.tv_sec*1000 + tv.tv_usec/1000; // 毫秒
	//return tv.tv_sec*1000000 + tv.tv_usec;   // 微妙
}
