#include <rfcomm.h>

#define BACK_LOG  5

int io_rfcomm_server(io_loop_t *loop, io_handle_t *handle, bdaddr_t *addr, uint8_t ch) {
   int fd;
   struct sockaddr_rc loc_addr = { 0 };

   if((fd = socket(AF_BLUETOOTH, SOCK_STREAM | SOCK_NONBLOCK, BTPROTO_RFCOMM)) == -1)
      return -1;

   loc_addr.rc_family = AF_BLUETOOTH;
   loc_addr.rc_bdaddr = *addr;
   loc_addr.rc_channel = ch;

   if(bind(fd, (struct sockaddr*)&loc_addr, sizeof(loc_addr)) == -1)
      goto rfcomm_server_fail;

   if(listen(fd, BACK_LOG) == -1)
      goto rfcomm_server_fail; 
   
   if(io__handle_init(handle, loop, fd) == -1) 
      goto rfcomm_server_fail;

   return 0;

rfcomm_server_fail:
   close(fd);
   return -1;
}

sdp_session_t* io_rfcomm_advertise(uint8_t ch, const char *name, const char *desc, const uint32_t *uuid128) {
uuid_t root_uuid, rfcomm_uuid, svc_uuid, svc_class_uuid, l2cap_uuid;
   sdp_list_t *rfcomm_list = 0,
              *root_list = 0,
              *proto_list = 0,
              *access_proto_list = 0,
              *svc_class_list = 0,
              *profile_list = 0,
              *l2cap_list = 0; //*l2cap_list
   sdp_data_t *channel = 0;
   sdp_profile_desc_t profile;
   sdp_record_t record = { 0 };
   sdp_session_t *session = 0;

   //set general service ID
   sdp_uuid128_create(&svc_uuid, &uuid128);
   sdp_set_service_id(&record, svc_uuid);

   // set the service class
   sdp_uuid16_create(&svc_class_uuid, SERIAL_PORT_SVCLASS_ID);
   svc_class_list = sdp_list_append(0, &svc_class_uuid);
   sdp_set_service_classes(&record, svc_class_list);

   // set the Bluetooth profile information
   sdp_uuid16_create(&profile.uuid, SERIAL_PORT_PROFILE_ID);
   profile.version = 0x0100;
   profile_list = sdp_list_append(0, &profile);
   sdp_set_profile_descs(&record, profile_list);

   // make the service record publicly browsable
   sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
   root_list = sdp_list_append(0, &root_uuid);
   sdp_set_browse_groups( &record, root_list );

   //set l2cap information
   sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
   l2cap_list = sdp_list_append( 0, &l2cap_uuid );
   proto_list = sdp_list_append( 0, l2cap_list );

   // register rfcomm channel for rfcomm sockets
   sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
   channel = sdp_data_alloc(SDP_UINT8, &ch);
   rfcomm_list = sdp_list_append( 0, &rfcomm_uuid );
   sdp_list_append( rfcomm_list, channel );
   sdp_list_append( proto_list, rfcomm_list );

   access_proto_list = sdp_list_append(0, proto_list);
   sdp_set_access_protos(&record, access_proto_list);

   //set name, provider, description
   sdp_set_info_attr(&record, name, "", desc);

   //connect to local SDP server, register service record, and disconnect
   session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, 0);
   sdp_record_register(session, &record, 0);

   //cleanup
   sdp_data_free(channel);
   sdp_list_free(l2cap_list, 0);
   sdp_list_free( rfcomm_list, 0 );
   sdp_list_free( root_list, 0 );
   sdp_list_free( access_proto_list, 0 );

   return session;
}

int io_rfcomm_unadvertise(sdp_session_t *session) {
   sdp_close(session);
   return 0;
}

int io_rfcomm_accept(io_handle_t *server, io_handle_t *client, char *addr, int len) {
   struct sockaddr_rc sockaddr;
   int fd;
   socklen_t socklen = sizeof(sockaddr);

   if((fd = accept(server->fd, (struct sockaddr*)&sockaddr, &socklen)) == -1) {
      return -1;
   }

   ba2str(&sockaddr.rc_bdaddr, addr);
   if(io__handle_init(client, server->loop, fd) == -1) {
      close(fd);
      return -1;
   }

   return 0;
}

int io_rfcomm_close(io_handle_t *handle) {
   io__handle_close(handle);
   close(handle->fd);
}
