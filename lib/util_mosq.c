/*
Copyright (c) 2009,2010, Roger Light <roger@atchoo.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of mosquitto nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <assert.h>
#include <string.h>
#include <time.h>

#include <mosquitto.h>
#include <memory_mosq.h>
#include <net_mosq.h>
#include <send_mosq.h>
#include <util_mosq.h>

#ifndef WITH_BROKER
void _mosquitto_check_keepalive(struct mosquitto *mosq)
{
	assert(mosq);
	if(mosq->core.sock != INVALID_SOCKET && time(NULL) - mosq->core.last_msg_out >= mosq->core.keepalive){
		if(mosq->core.state == mosq_cs_connected){
			_mosquitto_send_pingreq(mosq);
		}else{
			_mosquitto_socket_close(&mosq->core);
		}
	}
}
#endif

/* Convert ////some////over/slashed///topic/etc/etc//
 * into some/over/slashed/topic/etc/etc
 */
int _mosquitto_fix_sub_topic(char **subtopic)
{
	char *fixed = NULL;
	char *token;

	assert(subtopic);
	assert(*subtopic);

	/* size of fixed here is +1 for the terminating 0 and +1 for the spurious /
	 * that gets appended. */
	fixed = _mosquitto_calloc(strlen(*subtopic)+2, 1);
	if(!fixed) return MOSQ_ERR_NOMEM;

	if((*subtopic)[0] == '/'){
		fixed[0] = '/';
	}
	token = strtok(*subtopic, "/");
	while(token){
		strcat(fixed, token);
		strcat(fixed, "/");
		token = strtok(NULL, "/");
	}

	fixed[strlen(fixed)-1] = '\0';
	_mosquitto_free(*subtopic);
	*subtopic = fixed;
	return MOSQ_ERR_SUCCESS;
}

uint16_t _mosquitto_mid_generate(struct _mosquitto_core *core)
{
	assert(core);

	core->last_mid++;
	if(core->last_mid == 0) core->last_mid++;
	
	return core->last_mid;
}
