#include "LSA.h"

LSA *newLSA(uint32_t senderID, uint32_t seqNo)
{
    LSA *newObj = malloc(sizeof(LSA));
    newObj->dest = NULL;
    newObj->version = 1;
    newObj->TTL = 32;
    newObj->senderID = senderID;
    newObj->seqNo = seqNo;
    newObj->type = 0;
    return newObj;

}
void replaceLSA(LSA **ptr, LSA *newLSA)
{
    freeLSA(*ptr);
    *ptr = newLSA;
}

void incLSASeq(LSA *lsa)
{
    lsa->seqNo++;
}

void setLSAAck(LSA *lsa)
{
    lsa->type = 1;
}

int hasLSAAck(LSA *lsa)
{
    return lsa->hasACK;
}

void gotLSAAck(LSA *lsa)
{
    lsa->hasACK = 1;
}

int isLSAAck(LSA *lsa)
{
    return lsa->type == 1;
}

void decLSATTL(LSA *lsa)
{
    lsa->TTL--;
}

uint8_t getLSATTL(LSA *lsa)
{
    return lsa->TTL;
}

void setLSADest(LSA *lsa, char *dest, int port)
{
    lsa->port = port;
    lsa->dest = malloc(strlen(dest) + 1);
    strcpy(lsa->dest, dest);
}

LSA *headerLSAfromLSA(LSA *lsa)
{
    LSA *newObj = mallco(sizeof(LSA));
    newObj->dest = NULL;
    newObj->port = 0;
    newObj->src = lsa->src;
    newObj->version = lsa->version;
    newObj->TTL = lsa->TTL;
    newObj->type = lsa->type;
    newObj->senderID = lsa->senderID;
    newObj->seqNo = lsa->seqNo;
    newObj->numLink = 0;
    newObj->numObj = 0;
    newObj->listLink = NULL;
    newObj->listObj = NULL;
    return newObj;
}

LSA *LSAfromLSA(LSA *lsa)
{
    int size = sizeof(uint32_t) * lsa->numLink;
    LSA *newObj = headerLSAfromLSA(lsa);
    newObj->numLink = lsa->numLink;
    newObj->numObj = lsa->numObj;
    newObj->listLink = malloc(size);
    memcpy(newObj->listLink, lsa->listLink, size);
    newObj->listObj = copyList(lsa->listObj);

    return newObj;
}

LSA *LSAfromBuffer(char *buf, ssize_t length)
{
    int i;
    char *ptr;
    uint8_t version, TTL;
    uint16_t type;
    uint32_t senderID, seqNo, numLink, numObj;
    version = *(uint8_t *)buf;
    TTL = ntohs(*(uint8_t *)(buf + 1));
    type = ntohs(*(uint16_t *)(buf + 2));

    senderID = ntohl(*(uint32_t *)(buf + 4));
    seqNo = ntohl(*(uint32_t *)(buf + 8));
    numLink = ntohl(*(uint32_t *)(buf + 12));
    numObj = ntohl(*(uint32_t *)(buf + 16));

    LSA *newObj = malloc(sizeof(LSA));
    newObj->version = version;
    newObj->TTL = TTL;
    newObj->type = type;
    newObj->senderID = senderID;
    newObj->seqNo = seqNo;
    newObj->numLink = numLink;
    newObj->numObj = numObj;
    gettimeofday(newObj->timestamp, NULL);
    /* Get node ID */
    newObj->listLink = malloc(sizeof(uint32_t) * numLink);
    buf = buf + 20;
    for(i = 0; i < numLink; i++) {
        listLink[i] = ntohl(*(uint32_t *)(buf));
        buf += sizeof(uint32_t);
    }
    /* Get object list */
    newObj->listObj = malloc(sizeof(DLL));
    initList(newObj->listObj, compareString, freeString, NULL, copyString);

    for(i = 0; i < numObj; i++) {
        ptr = calloc(strlen(buf) + 1, 1);
        strcpy(ptr, buf);
        insertNode(newObj->listObj, ptr);
        buf += strlen(buf) + 1;
    }
    return newObj;
}

void LSAtoBuffer(LSA *lsa, char **buf, int *bufSize)
{
    int size = 20 + lsa->numLink * sizeof(uint32_t);
    int i;
    char *ptr;
    char *buffer = malloc(size);
    *buf = NULL;
    *bufSize = 0;
    memcpy(buffer, &(lsa->version), 1);
    memcpy(buffer + 1, &(lsa->TTL), 1);
    memcpy(buffer + 2, &htons(lsa->type), 2);
    memcpy(buffer + 4, &htonl(lsa->senderID), 4);
    memcpy(buffer + 8, &htonl(lsa->seqNo), 4);
    memcpy(buffer + 12, &htonl(lsa->numLink), 4);
    memcpy(buffer + 16, &htonl(lsa->numObj), 4);
    ptr = buffer + 20;
    for(i = 0; i < lsa->numLink; i++) {
        memcpy(ptr + i * 4, &htonl(lsa->listLink[i]), 4);
    }
    ptr += i * 4;

    for(i = 0; i < lsa->numObj; i++) {
        char *objPtr = getNodeDataAt(lsa->listObj, i);
        size = strlen(objPtr) + 1;
        buffer = realloc(buffer, ptr - buffer + size);
        strcpy(ptr, objPtr);
        ptr += size;
    }
    *buf = buffer;
    *bufSize = ptr - buffer;
}
