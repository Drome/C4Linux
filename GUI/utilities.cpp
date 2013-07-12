#include "utilities.h"

#include <iostream>

QString heuristicStrEncoding(char *str) {
    UErrorCode status = U_ZERO_ERROR;
    UCharsetDetector *csDet = ucsdet_open(&status);
    ucsdet_setText(csDet, str, -1, &status);
    int32_t matchCount; const UCharsetMatch **matches = ucsdet_detectAll(csDet, &matchCount, &status);
    for(int i=0; i<matchCount; i++) {
        const char * csName = ucsdet_getName(matches[i], &status);
        if((!strcmp(csName, "ISO-8859-1")) || (!strcmp(csName, "UTF-8"))) {
            UErrorCode tStat = U_ZERO_ERROR;
            int32_t len = ucsdet_getUChars(matches[i], NULL, 0, &tStat);
            UChar * buff = new UChar[len+1];
            ucsdet_getUChars(matches[i], buff, len+1, &status);
            if(U_FAILURE(status)) {
                delete[] buff;
                return QString(str);
            }
            QString ret = QString::fromUtf16(buff);
            delete[] buff;
            return ret;
        }
        else continue;
    }
    /*std::cout << "CS Detection Matches for " << str << ": " << matchCount << std::endl;
    for(int i=0; i<matchCount; i++) {
        UErrorCode tStat = U_ZERO_ERROR;
        const char * csName = ucsdet_getName(matches[i], &tStat);
        std::cout << (U_SUCCESS(tStat) ? csName : u_errorName(tStat)) << std::endl;
    }
    std::cout.flush();*/
    return QString(str);
}

void unpackC4GroupTo(const QString groupPath, const QString targetPath, bool createPath) {
    FILE * f = fopen(groupPath.toStdString().c_str(), "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    unsigned char * buff = new unsigned char[size];
    fread(buff, 1, size, f);
    fclose(f);
    C4GroupFile_t *grp = C4GROUP_unpack(buff, size, true, true, groupPath.split("/").last().toStdString().c_str());
    delete[] buff;
    unpackC4GroupTo(grp, targetPath, createPath);
}

void unpackC4GroupTo(const C4GroupFile_t *grp, const QString targetPath, bool createPath) {
    if(createPath) mkdir(targetPath.toStdString().c_str(), 0775);
    for(unsigned int i=0; i<grp->header.fileCount; i++) {
        C4File_t f = grp->files[i];
        QString fpath = targetPath; fpath.append("/"); fpath.append(heuristicStrEncoding(f.name));
        FILE * rf = fopen(fpath.toStdString().c_str(), "wb");
        fwrite(f.content, 1, f.size, rf);
        fclose(rf);
    }
    for(unsigned int i=0; i<grp->header.groupCount; i++) {
        C4GroupFile_t * subGrp = grp->groupFiles[i];
        QString dpath = targetPath; dpath.append("/"); dpath.append(heuristicStrEncoding(subGrp->header.name));
        mkdir(dpath.toStdString().c_str(), 0775);
        unpackC4GroupTo(subGrp, dpath);
    }
}
