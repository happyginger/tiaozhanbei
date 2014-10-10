#ifndef _WEIBO_H
#define _WEIBO_H

unsigned char post_weibo_update(char* weibo);
unsigned char post_weibo_upload(char* id, char* pwd, char* weibo, char* pic, uint16 picLen);
#endif
