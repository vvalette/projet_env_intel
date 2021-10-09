/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/app_ethernet.c 
  * @author  MCD Application Team
  * @brief   Ethernet specefic module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "main.h"
#include "lwip/dhcp.h"
#include "app_ethernet.h"
#include "ethernetif.h"
#include "lcd_log.h"
#include "mqtt.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define 	MQTT_PORT_TEST   1883
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef USE_DHCP
#define MAX_DHCP_TRIES  4
__IO uint8_t DHCP_state = DHCP_OFF;
#endif
static int inpub_id;
/* Private function prototypes -----------------------------------------------*/
void example_do_connect(mqtt_client_t *client);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_sub_request_cb(void *arg, err_t result);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);


/* Private functions ---------------------------------------------------------*/
void example_do_connect(mqtt_client_t *client)
{
  struct mqtt_connect_client_info_t ci;
  err_t err;

  /* Setup an empty client info structure */
  memset(&ci, 0, sizeof(ci));
  //memset(&client, 0, sizeof(client));
  /* Minimal amount of information required is client identifier, so set it here */
  char snum[10];
  char ident[20] = "AYk";

  itoa(rand() % 20, snum, 10);
  strcat(ident,snum);
  ci.client_id = ident;
  LCD_UsrLog ("id = %s\n", ci.client_id);
  /* Initiate client and connect to server, if this fails immediately an error code is returned
     otherwise mqtt_connection_cb will be called with connection result after attempting
     to establish a connection with the server.
     For now MQTT version 3.1.1 is always used */
  ip_addr_t adr;
  IP4_ADDR(&adr,18,194,65,151);
  //IP4_ADDR(&adr,192,168,1,122);
  //12C24197
  //C0A8016A
  err = mqtt_client_connect(client,&adr,1883, mqtt_connection_cb, 0, &ci);

  /* For now just print the result code if something goes wrong*/
  if(err != ERR_OK) {
	  LCD_UsrLog ("mqtt_connect return %d\n", err);
  }
}
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
  err_t err;
  err_t err1;
  err_t err2;
  if(status == MQTT_CONNECT_ACCEPTED) {
	  LCD_UsrLog ("mqtt_connection_cb: Successfully connected\n");

	    /* Setup callback for incoming publish requests */
	    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);
	    /* Subscribe to a topic named "subtopic" with QoS level 1, call mqtt_sub_request_cb with result */
	    err = mqtt_subscribe(client, "sensor_data/hum", 0, mqtt_sub_request_cb, arg);

	    err1 = mqtt_subscribe(client, "sensor_data/bat", 0, mqtt_sub_request_cb, arg);

	    err2 = mqtt_subscribe(client, "sensor_data/tem", 0, mqtt_sub_request_cb, arg);



    if(err != ERR_OK) {
    	LCD_UsrLog ("mqtt_subscribe return: %d\n", err);
    }
  } else {
	  LCD_UsrLog ("mqtt_connection_cb: Disconnected, reason: %d\n", status);

    /* Its more nice to be connected, so try to reconnect */
    //example_do_connect(client);
  }
}
static void mqtt_sub_request_cb(void *arg, err_t result)
{
  /* Just print the result code here for simplicity,
     normal behaviour would be to take some action if subscribe fails like
     notifying user, retry subscribe or disconnect from server */
	//LCD_UsrLog ("Subscribe result: %d\n", result);
}
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
	//LCD_UsrLog ("Incoming publish at topic %s with total length %u\n", topic, (unsigned int)tot_len);

  /* Decode topic string into a user defined reference */
  if(strcmp(topic, "sensor_data/hum") == 0) {
    inpub_id = 0;
  } else if(strcmp(topic, "sensor_data/bat") == 0) {
    /* All topics starting with 'A' might be handled at the same way */
    inpub_id = 1;
  } else if(strcmp(topic, "sensor_data/tem") == 0) {
    /* For all other topics */
    inpub_id = 2;
  }
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
	//LCD_UsrLog ("Incoming publish payload with length %d, flags %u\n", len, (unsigned int)flags);

  if(flags & MQTT_DATA_FLAG_LAST) {
    /* Last fragment of payload received (or whole part if payload fits receive buffer
       See MQTT_VAR_HEADER_BUFFER_LEN)  */
	  //LCD_UsrLog ("Temp: %s\n", MQTT_DATA_FLAG_LAST);

	  if(inpub_id == 0){
		  LCD_UsrLog ("humidity: %s\n", (const char *)data);
	  }
	  if(inpub_id == 1){
		  LCD_UsrLog ("batterye_power: %s\n", (const char *)data);
	  	  }
	  if(inpub_id == 2){
		  LCD_UsrLog ("temperature: %s\n", (const char *)data);
	  	  }
  }
    /* Call function or do action depending on reference, in this case inpub_id */
   else {
    /* Handle fragmented payload, store in buffer, write to file or whatever */
  }
}

/**
  * @brief  Notify the User about the network interface config status
  * @param  netif: the network interface
  * @retval None
  */
void User_notification(struct netif *netif) 
{
  if (netif_is_up(netif))
  {
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_START;
#else
    uint8_t iptxt[20];
    sprintf((char *)iptxt, "%s", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
    LCD_UsrLog ("Static IP address: %s\n", iptxt);
#endif /* USE_DHCP */
  }
  else
  {  
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_LINK_DOWN;
#endif  /* USE_DHCP */
    LCD_UsrLog ("The network cable is not connected \n");
  } 
}

#ifdef USE_DHCP
/**
  * @brief  DHCP Process
  * @param  argument: network interface
  * @retval None
  */
void DHCP_thread(void const * argument)
{
  struct netif *netif = (struct netif *) argument;
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  struct dhcp *dhcp;
  uint8_t iptxt[20];
  
  for (;;)
  {
    switch (DHCP_state)
    {
    case DHCP_START:
      {
        ip_addr_set_zero_ip4(&netif->ip_addr);
        ip_addr_set_zero_ip4(&netif->netmask);
        ip_addr_set_zero_ip4(&netif->gw);       
        dhcp_start(netif);
        DHCP_state = DHCP_WAIT_ADDRESS;
        LCD_UsrLog ("  State: Looking for DHCP server ...\n");
      }
      break;
      
    case DHCP_WAIT_ADDRESS:
      {                
        if (dhcp_supplied_address(netif)) 
        {

          sprintf((char *)iptxt, "%s", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));   
          LCD_UsrLog ("IP address assigned by a DHCP server: %s\n", iptxt);
          DHCP_state = DHCP_ADDRESS_ASSIGNED;
        }
        else
        {
          dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
    
          /* DHCP timeout */
          if (dhcp->tries > MAX_DHCP_TRIES)
          {
            DHCP_state = DHCP_TIMEOUT;
            
            /* Stop DHCP */
            dhcp_stop(netif);
            
            /* Static address used */
            IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
            IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
            IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
            netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));
            
            sprintf((char *)iptxt, "%s", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
            LCD_UsrLog ("DHCP Timeout !! \n");
            LCD_UsrLog ("Static IP address: %s\n", iptxt);  
          }
        }
      }
      break;
  case DHCP_LINK_DOWN:
    {
      /* Stop DHCP */
      dhcp_stop(netif);
      DHCP_state = DHCP_OFF; 
    }
    break;
  case DHCP_ADDRESS_ASSIGNED:
      {
          mqtt_client_t *client = mqtt_client_new();
          if(client != NULL) {
              example_do_connect(client);
              osDelay(2000);
            }
          DHCP_state = NULL;
      }
      break;
    default: break;
    }
    
    /* wait 250 ms */
    osDelay(250);
  }
}
#endif  /* USE_DHCP */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
