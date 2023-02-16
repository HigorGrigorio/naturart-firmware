/**
 * @file web-server.h
 * @brief Configure the web server
 * @details This file contains the functions to configure the web server.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2020-05-01
 *
 */

#ifndef _WebServerConfig_h_
#define _WebServerConfig_h_

/**
 * @brief Web server local ip address
 */
#define WEB_SERVER_LOCAL_IP IPAddress(192, 168, 1, 1)

/**
 * @brief Web server gateway ip address
 */
#define WEB_SERVER_GATEWAY IPAddress(192, 168, 1, 1)

/**
 * @brief Web server subnet mask
 */
#define WEB_SERVER_SUBNET IPAddress(255, 255, 255, 0)

/**
 * @brief Web server port
*/
#define WEB_SERVER_AP_SSID "Configure o sensor Naturart"

#endif // !_WebServerConfig_h_