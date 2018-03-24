#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#define DH_ERR_NOERR      0
#define DH_ERR_NOTFOUND  -1

int dhGetRegistry(const char *name,void *data,int size);
int dhSetRegistry(const char *name,const void *data,int size);

typedef void (*DH_REG_CALLBACK)(const char *name,const char *val,int size);

int dhRegistCallback(const char *name,DH_REG_CALLBACK *func);

void dhLoadRegistry(void *buf);
int dhSaveRegistry(void *buf);

#endif
