#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#define DH_ERR_NOERR      0
#define DH_ERR_NOTFOUND  -1

int GetRegistryDWORD(const char *name,u32 *data);
int GetRegistry(const char *name,void *data,int size);
int SetRegistry(const char *name,const void *data,int size);
int SetRegistryStr(const char *name,const void *data);
int SetRegistryDWORD(const char *name,u32 data);
int SetRegistryInt(const char *name,int data);

u32 get_value(char *name);
void set_value(char *name,int val);

void InitRegistry(void);
void InitRegistryLang(void);

#endif
