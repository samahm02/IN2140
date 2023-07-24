
#include "recordFromFormat.h"
 
#include <arpa/inet.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
 

Record *XMLtoRecord(char *buffer, int bufSize, int *bytesread)
{
    Record *record = newRecord();
    int posisjon = 0;
    *bytesread = 0;
    bool ferdig = false;

    while ((posisjon < bufSize) && !ferdig)
    {
        if (buffer[posisjon] == '<')
        {
            posisjon++;

            int i = 0, y = 0;
            char *xmlTagName = malloc(20 * sizeof(char));
            char *xmlValue = malloc(2500 * sizeof(char));

            if (xmlTagName == NULL) {
                fprintf(stderr, "Kunne ikke tildele minne for xmlTagName\n");
                return NULL;
            }
    
            if (xmlValue == NULL) {
                fprintf(stderr, "Kunne ikke tildele minne for brukernavn xmlValue\n");
                free(xmlTagName);
                return NULL;
            }
            
            memset(xmlTagName, '\0', 20);
            memset(xmlValue, '\0', 2500);
            bool freed = false;

            while (buffer[posisjon] != 61)
            {
                if (buffer[posisjon] == '>')
                {
                    if (strcmp(xmlTagName, "/record") == 0)
                    {
                        ferdig = true;
                    }
                    free(xmlTagName);
                    free(xmlValue);
                    freed = true;
                    break;
                }
 
                xmlTagName[i] = buffer[posisjon];
                i++;
                posisjon++;
            }

            if (!freed && buffer[posisjon] != '>') 
            {
                posisjon++;
                posisjon++;

                while (buffer[posisjon] != '"')
                {
                    xmlValue[y] = buffer[posisjon];
                    y++;
                    posisjon++;
                }
                xmlValue[y] = '\0';
                
                if (strcmp(xmlTagName, "source") == 0)
                {
                    setSource(record, xmlValue[0]);
                }
                if (strcmp(xmlTagName, "dest") == 0)
                {
                    setDest(record, xmlValue[0]);
                }
                if (strcmp(xmlTagName, "username") == 0)
                {
                    setUsername(record, xmlValue);
                }
            
                if (strcmp(xmlTagName, "id") == 0)
                {
                    uint32_t resultat = strtoul(xmlValue, NULL, 10);
                    setId(record, resultat);
                }
                if (strcmp(xmlTagName, "group") == 0)
                {
                    uint32_t resultat = strtoul(xmlValue, NULL, 10);
                    setGroup(record, resultat);
                }
                if (strcmp(xmlTagName, "semester") == 0)
                {
                    int resultat = atoi(xmlValue);
                    setSemester(record, resultat);
                }
                if (strcmp(xmlTagName, "grade") == 0)
                {
                    if (strcmp(xmlValue, "PhD") == 0)
                    {
                        setGrade(record, Grade_PhD);
                    }
                    else if (strcmp(xmlValue, "Master") == 0)
                    {
                        setGrade(record, Grade_Master);
                    }
                    else if (strcmp(xmlValue, "Bachelor") == 0)
                    {
                        setGrade(record, Grade_Bachelor);
                    }
                    else if (strcmp(xmlValue, "None") == 0)
                    {
                        setGrade(record, Grade_None);
                    }
                }
                if (strcmp(xmlTagName, "course") == 0)
                {
                    if (strcmp(xmlValue, "IN1000") == 0)
                    {
                        setCourse(record, Course_IN1000);
                    }
                    else if (strcmp(xmlValue, "IN1010") == 0)
                    {
                        setCourse(record, Course_IN1010);
                    }
                    else if (strcmp(xmlValue, "IN1020") == 0)
                    {
                        setCourse(record, Course_IN1020);
                    }
                    else if (strcmp(xmlValue, "IN1030") == 0)
                    {
                        setCourse(record, Course_IN1030);
                    }
                    else if (strcmp(xmlValue, "IN1050") == 0)
                    {
                        setCourse(record, Course_IN1050);
                    }
                    else if (strcmp(xmlValue, "IN1060") == 0)
                    {
                        setCourse(record, Course_IN1060);
                    }
                    else if (strcmp(xmlValue, "IN1080") == 0)
                    {
                        setCourse(record, Course_IN1080);
                    }
                    else if (strcmp(xmlValue, "IN1140") == 0)
                    {
                        setCourse(record, Course_IN1140);
                    }
                    else if (strcmp(xmlValue, "IN1150") == 0)
                    {
                        setCourse(record, Course_IN1150);
                    }
                    else if (strcmp(xmlValue, "IN1900") == 0)
                    {
                        setCourse(record, Course_IN1900);
                    }
                    else if (strcmp(xmlValue, "IN1910") == 0)
                    {
                        setCourse(record, Course_IN1910);
                    }
                }

            }
 
            if (!freed)
            {
                free(xmlTagName);
                free(xmlValue);
            }
        }
        else
        {
            posisjon++;
        }
    }

    if (ferdig)
    {
        *bytesread = posisjon;
        return record;
    }
    else
    {
        deleteRecord(record);
        return NULL;
    }
}


Record* BinaryToRecord(char* buffer, int bufSize, int* bytesread)
{
    Record* record = newRecord();
    int posisjon = 0;

    uint8_t attributeFlags = buffer[posisjon];
    posisjon++;

    if (posisjon > bufSize) {
        deleteRecord(record);
        return NULL;
    }

    if (attributeFlags & FLAG_SRC) {
        if (posisjon < bufSize) {
            setSource(record, buffer[posisjon]);
            posisjon++;
        } else {
            deleteRecord(record);
            return NULL;
        }
    }

    if (attributeFlags & FLAG_DST) {
        if (posisjon < bufSize) {
            setDest(record, buffer[posisjon]);
            posisjon++;
        } else {
            deleteRecord(record);
            return NULL;
        }
    }

    if (attributeFlags & FLAG_USERNAME) {
        uint32_t navnLengde;
        if (posisjon + (int)sizeof(uint32_t) <= bufSize) {
            memcpy(&navnLengde, &buffer[posisjon], sizeof(uint32_t));
            navnLengde = ntohl(navnLengde);
            posisjon += sizeof(uint32_t);
        } else {
            deleteRecord(record);
            return NULL;
        }

        if (posisjon + (int)navnLengde <= bufSize) {
            char* brukernavn = calloc(navnLengde + 1, sizeof(char));
            if (brukernavn == NULL) {
                fprintf(stderr, "%s:%d Kunne ikke tildele heap minne for brukernavn\n", __FILE__, __LINE__);
                deleteRecord(record);
                return NULL;
            }
            memcpy(brukernavn, &buffer[posisjon], navnLengde);
            setUsername(record, brukernavn);
            free(brukernavn);
            posisjon += navnLengde;
        } else {
            fprintf(stderr, "%s:%d Buffer overflow under innlesing av brukernavn\n", __FILE__, __LINE__);
            deleteRecord(record);
            return NULL;
        }
    }

    if (attributeFlags & FLAG_ID) {
        if (posisjon + (int)sizeof(uint32_t) <= bufSize) {
            uint32_t id;
            memcpy(&id, &buffer[posisjon], sizeof(uint32_t));
            setId(record, ntohl(id));
            posisjon += sizeof(uint32_t);
        } else {
            deleteRecord(record);
            return NULL;
        }
    }

    if (attributeFlags & FLAG_GROUP) {
        if (posisjon + (int)sizeof(uint32_t) <= bufSize) {
            uint32_t group;
            memcpy(&group, &buffer[posisjon], sizeof(uint32_t));
            setGroup(record, ntohl(group));
            posisjon += sizeof(uint32_t);
        } else {
            deleteRecord(record);
            return NULL;
        }
    }

    if (attributeFlags & FLAG_SEMESTER) {
        if (posisjon < bufSize) {
            setSemester(record, buffer[posisjon]);
            posisjon++;
        } else {
            deleteRecord(record);
            return NULL;
        }
    }

    if (attributeFlags & FLAG_GRADE) {
        if (posisjon < bufSize) {
            Grade grade = buffer[posisjon];
            setGrade(record, grade);
            posisjon++;
        } else {
            deleteRecord(record);
            return NULL;
        }
    }

    if (attributeFlags & FLAG_COURSES) {
        if (posisjon + (int)sizeof(uint16_t) <= bufSize) {
            uint16_t coursesFlags;
            memcpy(&coursesFlags, &buffer[posisjon], sizeof(uint16_t));
            coursesFlags = ntohs(coursesFlags);
            posisjon += sizeof(uint16_t);

            for (int i = Course_IN1000; i <= Course_IN1910; i <<= 1) {
                if (coursesFlags & i) {
                    setCourse(record, i);
                }
            }
        } else {
            deleteRecord(record);
            return NULL;
        }
    }

    *bytesread = posisjon;

    if (posisjon == 1) {
        deleteRecord(record);
        return NULL;
    }

    return record;
}

