#ifndef _NETWORK_H_
#define _NETWORK_H_

/**
 * @file
 * Network services.
 */

/**
 * Begin Access Point.
 */
void ap_begin();

/**
 * End Access Point.
 */
void ap_end();

/**
 * Begin mDNS responder.
 */
void mdns_begin();

/**
 * End mDNS responder.
 */
void mdns_end();

#endif
