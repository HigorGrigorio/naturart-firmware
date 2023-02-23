/**
 * @file dns-server.h
 * @brief Configure the dns server
 * @details This file contains the functions to configure the dns server.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 *
 */

#ifndef _DnsServer_h_
#define _DnsServer_h_

#include <DNSServer.h>

#include <Nonnull.h>

#include <wifi-credentials.h>

#ifndef DNS_PORT
#warning "DNS_PORT is not defined, using 80 as default"
#define DNS_PORT 80
#endif // !DNS_PORT

#ifndef DNS_SERVER_IP
#warning "DNS_SERVER_IP is not defined, using 192.168.1.1 as default"
#define DNS_SERVER_IP IPAddress(192, 168, 1, 1)
#endif // !DNS_SERVER_IP

#ifndef DNS_SERVER_DOMAIN
#warning "DNS_SERVER_DOMAIN is not defined, using 'Configure o sensor Naturart' as default"
#define DNS_SERVER_DOMAIN "Configure o sensor Naturart"
#endif // !DNS_SERVER_DOMAIN

auto ConfigureDNSServer(Nonnull<DNSServer *> dnsServer)
{
    dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer->start(DNS_PORT, DNS_SERVER_DOMAIN, DNS_SERVER_IP);
}

#endif // !_DnsServer_h_