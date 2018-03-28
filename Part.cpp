//============================================================================
// Name        : SharedHaplotypeMatching.cpp
// Author      : Rasool
// Version     :
// Copyright   :
// Description :
//1. Call ReadFiles.cpp. It checks the integrity of the file.
//2.
//============================================================================
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string.h>
#include <cstdio>
#include <stdio.h>
#include <zlib.h>
#include <limits.h> /* for PATH_MAX */
#include <string>
#include "ReadFiles.hpp"
#include "SortFile.hpp"
#include "Part.hpp"
#include "VectorClass.hpp"

/*compressFile is NOT a member function of the class. */
int compressFile(FILE *writeDeflatedOutFile, const char * const compressed_outFileName)
{
    /* Buffer to hold data read */
    char buf[BUFSIZ] = { 0 };
    size_t bytes_read = 0;
    gzFile out = gzopen(compressed_outFileName, "wb");
    if (!out)
    {
        /* Handle error */
        fprintf(stderr, "Unable to open %s for writing\n", compressed_outFileName);
        exit(0);
        //return -1;
    }
    bytes_read = fread(buf, 1, BUFSIZ, writeDeflatedOutFile);
    while (bytes_read > 0)
    {
        int bytes_written = gzwrite(out, buf, bytes_read);
        if (bytes_written == 0)
        {
            int err_no = 0;
            fprintf(stderr, "Error during compression: %s", gzerror(out, &err_no));
            gzclose(out);
            exit(0);
            //return -1;
        }
        bytes_read = fread(buf, 1, BUFSIZ, writeDeflatedOutFile);
    }
    gzclose(out);
    //fclose(writeDeflatedOutFile);

    return 0;
}

part::part()
{
}
part::part(ReadFiles fileData)
{
    fileName=fileData.getInpFileName();
    ofilename=fileData.getOutFileName();
    //std::cout<<"Constructor "<<ofilename<<std::endl;
}
std::string part::quickSort_hapindex(part ind)
{
    SortFile compute;
    return compute.ComputeSorting(ind);	//Sort the file
}

void part::setFlags(Flags flag)
{
    FLAGS =	flag;
}


void part::computation_haplotype(part ind1)
{
    std::string eachi;	//for indexing
    std::stringstream ss;
    fileName=quickSort_hapindex(ind1);//Call sorting algo here
    std::ofstream writeInflatedOutFile;	/*Create the NON-compressed file*/
    writeInflatedOutFile.open ((ind1.ofilename+".txt").c_str());	//can also be ind2
    if (!writeInflatedOutFile.is_open())
    {
        std::cerr<<ofilename<<" cannot be created.. Program exiting"<<std::endl;
        exit(0);
    }
    std::ifstream f_indexing(fileName.c_str());	//Open file to create objects
    std::vector <VectorClass> vec;
    unsigned long int min_bp_dist = ind1.FLAGS.getMIN_BP_DISTANCE(); // To avoid recomputing everytime
    unsigned long int counter=0;
    while(getline(f_indexing,eachi))
    {
        counter++;
        if (!strcmp("h_ID chr hap st en hap_index root_pop",eachi.c_str()))	/*Correct .int file is opened*/
            continue;
        ss<<eachi;	//Copy the each line to stringstream
        ss>>ind1.h_ID;	/*get the h_ID i..e the name of the individual from .int file*/
        ss>>ind1.chr;
        ss>>ind1.hap;
        ss>>ind1.st;
        ss>>ind1.en;
        ss>>ind1.hap_index;
        ss>>ind1.root_pop;
        ss.str("");	/*Since one value is extracted, clear the stringstream*/
        ss.clear();	/*Clear state flags*/
        //std::ifstream f2(fileName.c_str()); /*restart inner loop from the top of the file each time*/
        vec.push_back(VectorClass(ind1.h_ID, ind1.chr, ind1.hap,ind1.st,ind1.en,ind1.hap_index,ind1.root_pop,counter));
    }	//Outer while loop
    f_indexing.close();
    std::string each,every;
    unsigned long int vec_size= vec.size();
    writeInflatedOutFile<<"ind1 "<<"ind2 "<<"chr "<<"start "<<"end "<<"bp_distance "<<"hap1 "<<"hap2"<<std::endl;
    for (unsigned long int i=0;i<vec_size;i++)
    {
        for (unsigned long int j=i;j<vec_size;j++)
        {
            // We have assumed that the chromosomes are in different files.
            // TODO: we should check the root_pop
            if (vec[i].hap_index == vec[j].hap_index)
            {
                unsigned long int sh_st = std::max(vec[i].st,vec[j].st);
                unsigned long int sh_en = std::min(vec[i].en,vec[j].en);
                unsigned long int sh_len = sh_en - sh_st + 1;
                if(sh_len >= min_bp_dist && sh_st <= sh_en)	// there is a shared part
                {
                    // first h_ID should be geq (>=) the second h_ID
                    if(vec[i].h_ID >= vec[j].h_ID) // it is possible to compare i with i
                    {
                        writeInflatedOutFile << vec[i].h_ID << " ";
                        writeInflatedOutFile << vec[j].h_ID << " ";
                        writeInflatedOutFile << vec[i].chr  << " ";
                        writeInflatedOutFile << sh_st << " ";
                        writeInflatedOutFile << sh_en << " ";
                        writeInflatedOutFile << sh_len << " "; // sh length
                        writeInflatedOutFile << vec[i].hap << " ";
                        writeInflatedOutFile << vec[j].hap << std::endl;
                    }
                    else // first h_ID should be geq (>=) the second h_ID
                    {
                        writeInflatedOutFile << vec[j].h_ID << " ";
                        writeInflatedOutFile << vec[i].h_ID << " ";
                        writeInflatedOutFile << vec[i].chr  << " ";
                        writeInflatedOutFile << sh_st << " ";
                        writeInflatedOutFile << sh_en << " ";
                        writeInflatedOutFile << sh_len << " "; // sh length
                        writeInflatedOutFile << vec[j].hap << " "; // the second hap
                        writeInflatedOutFile << vec[i].hap << std::endl;  // the first hap
                    }
                }
            } // if (vec[i].hap_index == vec[j].hap_index)
            else if (vec[i].hap_index <  vec[j].hap_index) // there should not be any other sh part
            {
                break;
            }
            else // impossible
            {
                std::cout<<vec[i].sn_no<<" "<<vec[j].sn_no<<std::endl;
                std::cout<<vec[i].h_ID<<" "<<vec[i].chr<<" "<<vec[i].hap<<" "<<vec[i].st<<" "<<vec[i].en<<" "<<vec[i].hap_index<<" "<<vec[i].root_hap<<std::endl;
                std::cout<<vec[j].h_ID<<" "<<vec[j].chr<<" "<<vec[j].hap<<" "<<vec[j].st<<" "<<vec[j].en<<" "<<vec[j].hap_index<<" "<<vec[j].root_hap<<std::endl<<std::endl;
                std::cerr<<"Error: wrong sorting... ERROR!!!"<<std::endl;
                exit(0);
            }

        }	//Inner for loop closed
    }	//Outer for loop closed


    if (remove(fileName.c_str())!=0)
    {

    }
    writeInflatedOutFile.close();	//close the file pointer
    /*Z-lib C- code call for compression*/
    char compressed_outFileName[PATH_MAX] = { 0 };
    FILE *writeDeflatedOutFile = fopen(((ind1.ofilename+".txt").c_str()), "r");

    if (writeDeflatedOutFile)
    {
        sprintf(compressed_outFileName, "%s.gz", ind1.ofilename.c_str());
        if (compressFile(writeDeflatedOutFile, compressed_outFileName) != 0)
        {
            fclose(writeDeflatedOutFile);
            //return -1;
        }
        fclose(writeDeflatedOutFile);
    }

    /*Remove the temporary .txt file*/

    if( remove( (ind1.ofilename+".txt").c_str()) != 0 )
    {
        std::cerr<< "Error deleting file"<<std::endl;
        exit(0);
    }
}//Function closed
