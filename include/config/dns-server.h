/**
 * @file dns-server.h
 * @brief Configure the dns server
 * @details This file contains the functions to configure the dns server.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-23-2
 *
 */

#ifndef _DnsServerConfig_h_
#define _DnsServerConfig_h_

/**
 * @brief DNS port
*/
#define DNS_PORT 53

/**
 * @brief DNS server IP.
 * @details This is the IP address of the DNS server. Use a instance of IPAddress class.
*/
#define DNS_SERVER_IP IPAddress(192, 168, 1, 1)

/**
 * @brief DNS server domain.
*/
#define DNS_SERVER_DOMAIN "Configure o sensor Naturart"

#endif // !_DnsServerConfig_h_