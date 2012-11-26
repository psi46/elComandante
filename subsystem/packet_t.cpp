/**
 * packet_t.cpp
 * Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * 14 Feb 2008
 */

#include "packet_t.h"

#include "error.h"

int packet_t::MAX_LENGTH = MAX_PACKETLEN;

packet_t::packet_t() {
	length = 0;
	#ifdef _WIN32
		memset(raw, 0, MAX_PACKETLEN);
		memset(&address, 0, sizeof(address));
	#else
		bzero(raw, MAX_PACKETLEN);
		bzero(&address, sizeof(address));
	#endif
	addrlen = 0;
	type = PKT_DEFAULTTYPE;
}


bool packet_t::setAddress(struct sockaddr_in addr, socklen_t len) {
	if (len < 0 || len > sizeof(address)) return false;
	memcpy(&address, &addr, len);
	addrlen = len;
	return true;
}

bool packet_t::isValid() const {
	if ( (length < 4) || (length > MAX_PACKETLEN ) ) { return false; }
	if ( (namelength > MAX_PACKETLEN-4) || (namelength > length-2) ) { return false; }
	if (addrlen < 0 || addrlen > sizeof(struct sockaddr)) { return false; }
	return true;
}

int packet_t::setName(const char* newname) {
	int buflen = strlen(newname);
	if ( buflen > MAX_PACKETLEN - 5 ) buflen = MAX_PACKETLEN - 5;	// FIXME: 5 is somewhat arbitrary
	char buffer[buflen];
	strncpy(buffer, newname, buflen);
	buffer[buflen]=0;
	for (int i=0; i<buflen; i++) {
		if ( buffer[i] == 13 || buffer[i] == 10 ) {
			buffer[i]=0;
			break;
		}
	}
	buflen = strlen(buffer);
	strncpy((char*)alldata, buffer, buflen+1);	// copy string and \0
	namelength=buflen+1;
	length=PKT_HEADLENGTH+namelength;
	
	return namelength;
}

char* packet_t::name() const {	// FIXME: this is unclean! const cannot be assured
	if (alldata[namelength-1] != 0) {
		eprintf("WARNING: Name of packet not terminated!");
	}
	// alldata[namelength-1]=0; // FIXME: conflicts with const
	return (char*)alldata;
}
int packet_t::namelen() const { return namelength; }

int packet_t::setData(const char* buffer, const int len) {
	int copylen = MAX_PACKETLEN-PKT_HEADLENGTH-namelength;	// PKT_HEADLENGTH + name
	if (len < copylen) copylen = len;
	memcpy(data(), buffer, copylen);

	length = PKT_HEADLENGTH + namelength + copylen;
	raw[length] = 0; // make \0 termination for printf

	return(length);
}

char* packet_t::data() const { return (char*)&(alldata[namelength]); }	// FIXME: this is unclean! const cannot be assured
int packet_t::datalen() const { return length-PKT_HEADLENGTH-namelength; }

char* packet_t::addrString() const {
	static char buffer[128];
	snprintf(buffer, 128, "%s:%d", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	return buffer;
}

void packet_t::print() const {
	//raw[length] = 0; // make \0 termination for printf // FIXME: conflicts with const, see setdata
	eprintf("addr=%s, length=%d, type=%d, namelen=%d, name='%s', data='%s', datalen=%d\n",
		addrString(), length, type, namelength, name(), data(), datalen());

	if ( 0 ) { // want raw output?
		eprintf("raw: ");
		for (int i=0; i<length; i++) {
			eprintf("%02X ", raw[i]);
		}
		eprintf("\n");
	}
}

int packet_t::pprintf(const char* format, ...) {	// print to packet data field
	char buffer[MAX_PACKETLEN];
	int txlen;
	// build string
	va_list ap;

	va_start(ap, format);
 	txlen = vsnprintf(buffer, MAX_PACKETLEN-(PKT_HEADLENGTH+namelen()+1), format, ap);
	setData(buffer, txlen);
	va_end(ap);

	// send packet FIXME: maybe dont do this here...
	//if ((txlen=sendto(sfd, raw, length, 0, (struct sockaddr*)&(address), sizeof(address))) < 0) {
	//	eperror("ERROR: local send() error");
	//}
	//if ( txlen < length ) {
	//	eprintf("WARNING: truncated packet(len=%d) to %d bytes.\n", length, txlen);
	//}
	return txlen;
	
}

