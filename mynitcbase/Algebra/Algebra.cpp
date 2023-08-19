#include "Algebra.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <iostream>

/* used to select all the records that satisfy a condition.
the arguments of the function are
* srcRel - the source relation we want to select from
* targetRel - the relation we want to select into. (ignore for now)
* attr - the attribute that the condition is checking
* op - the operator of the condition
* strVal - the value that we want to compare against (represented as a string)
*/
int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
{
    int srcRelId = OpenRelTable::getRelId(srcRel); // we'll implement this later
    if (srcRelId == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    // get the attribute catalog entry for attr, using AttrCacheTable::getAttrcatEntry()
    //    return E_ATTRNOTEXIST if it returns the error
    AttrCatEntry attrCatEntry;
    int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);

    if (ret == E_ATTRNOTEXIST)
        return E_ATTRNOTEXIST;

    // TODO: Convert strVal (string) to an attribute of data type NUMBER or STRING 
    int type = attrCatEntry.attrType;
    Attribute attrVal;
    if (type == NUMBER)
    {
        if (isNumber(strVal)) // the isNumber() function is implemented below
        { 
            attrVal.nVal = atof(strVal);
        }
        else
        {
            return E_ATTRTYPEMISMATCH;
        }
    }
    else if (type == STRING)
    {
        strcpy(attrVal.sVal, strVal);
    }

    // TODO: Select records from the source relation

    //* 1. Before calling the search function, reset the search to start from the first hit
    
    //* using RelCacheTable::resetSearchIndex()
    RelCacheTable::resetSearchIndex(srcRelId);

    //* 2. get relCatEntry using RelCacheTable::getRelCatEntry()
    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    /************************
    The following code prints the contents of a relation directly to the output
    console. Direct console output is not permitted by the actual the NITCbase
    specification and the output can only be inserted into a new relation. We will
    be modifying it in the later stages to match the specification.
    ************************/

   //* 3. Printing the actual record where a match occurred
    printf("|");
    for (int i = 0; i < relCatEntry.numAttrs; ++i)
    {
        // get attrCatEntry at offset i using AttrCacheTable::getAttrCatEntry()
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);

        printf(" %s |", attrCatEntry.attrName);
    }
    printf("\n");

    while (true)
    {
        RecId searchRes = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);

        if (searchRes.block != -1 && searchRes.slot != -1)
        {
            // get the record at searchRes using BlockBuffer.getRecord
            RecBuffer blockBuffer (searchRes.block);
            
            HeadInfo blockHeader;
            blockBuffer.getHeader(&blockHeader);

            Attribute recordBuffer [blockHeader.numAttrs];
            blockBuffer.getRecord(recordBuffer, searchRes.slot);   

            // TODO: print the attribute values in the same format as above
            printf("|");
            for (int i = 0; i < relCatEntry.numAttrs; ++i)
            {
                AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
                if (attrCatEntry.attrType == NUMBER)
                    printf(" %d |", (int)recordBuffer[i].nVal);
                else 
                    printf(" %s |", recordBuffer[i].sVal);

                // std:: cout << " " << (attrCatEntry.attrType == NUMBER ? 
                                        // recordBuffer[i].nVal : recordBuffer[i].sVal) << " |" ;
            }
            printf("\n");
        }
        else // (all records over)
            break;
    }

    return SUCCESS;
}

// will return if a string can be parsed as a floating point number
inline bool isNumber(char *str)
{
    int len;
    float ignore;
    /*
      sscanf returns the number of elements read, so if there is no float matching
      the first %f, ret will be 0, else it'll be 1

      %n gets the number of characters read. this scanf sequence will read the
      first float ignoring all the whitespace before and after. and the number of
      characters read that far will be stored in len. if len == strlen(str), then
      the string only contains a float with/without whitespace. else, there's other
      characters.
    */
    int ret = sscanf(str, "%f %n", &ignore, &len);
    return ret == 1 && len == strlen(str);
}
