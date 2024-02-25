#ifndef CHECKSUM_H
#define CHECKSUM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

uint16_t cksumIp(iphdr* pIpHead);
uint16_t cksumTcp(iphdr* pIpHead, tcphdr* pTcpHead);
uint16_t cksumUdp(iphdr* pIpHead, udphdr* pUdpHead);

#ifdef __cplusplus
}
#endif

#endif // CHECKSUM_H
