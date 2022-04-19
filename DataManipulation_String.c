/*search for a word and arrange some numbers task

  My approach was to handle the data by eliminating step by step the information we don't need.
  in this case I used the File handling tools provided by the C language (because it can create executable files)
  to store data as temporal files, then at the end delete the temporal files and just have one final Output.txt file with the sorted data
  and processing time printed in the first line of the file.

  Personal comments about the task:
  I decided to not use third party libraries, so I used the standard libraries for C in this case

searching specific word and arranging some numbers

  -Jersom López Serventi
*/

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>

int invertCmp(const void *a, const void *b);

int main()
{
   FILE *Input, *Output, *Temp1, *Temp2, *Temp3;
   char buf[512], buf2[512], stringToFind[]= "WORD TO SEARCH", filter[]=" ", filter2[]="\"", *string, *letterH;
   int i, j, counter=0;
   double runTime = 0.0;

        if((Input = fopen("Input.txt", "r")) == NULL)
            {
            printf("Error Reading File\n");
            exit(0);
            }

        if((Temp1 = fopen("Temp1.txt", "a+")) == NULL)
            {
            printf("Error Reading File\n");
            exit(0);
            }

      clock_t begin = clock();////start timer for processing data
        ////Remove the surrounding quotation marks
        ////These nested Loops are for removing the data that we are not interesting in, just keeping strings with the data we want (which have ":" and "porALPHA"
        while(fgets(buf, 512, Input) != NULL)
        {
            i=0;
                while(i<strlen(buf))
                {
                    if (buf[i]==':')
                    {
                        for (j=i; j<strlen(buf); j++)
                            buf[j]=buf[j+1];
                    } else i++;
                }
               string = strtok(buf, filter);

               while( string != NULL && strlen(string) != 0 )
               {
                  string = strtok(NULL, filter2);
                  if(string!=NULL && string !="\n")
                  {
                    if(strstr(string,stringToFind)!=NULL)fprintf(Temp1,"%s\n", string);
                  }
               }
            counter++;
        }
        fclose(Temp1);
        fclose(Input);

int *array, arraySize=0;
        if((Input = fopen("Temp1.txt", "r")) == NULL)
            {
            printf("Error Reading File\n");
            exit(0);
            }else
            {
                while(fgets(buf, 512, Input) != NULL){arraySize++;}///getting the number of lines of data to process
            }
            fclose(Input);

        if((Input = fopen("Temp1.txt", "r")) == NULL)
            {
            printf("Error Reading File\n");
            exit(0);
            }

        if((Temp2 = fopen("Temp2.txt", "a+")) == NULL)
            {
            printf("Error Reading File\n");
            exit(0);
            }
            array= (int *)malloc(sizeof(int)*arraySize);
        i=0;

        ///Isolating the digits that will be shifted to the end of the data line:
        //Move the substring between the character string PA and the first colon to the end of the data poin
        while(fgets(buf2, 512, Input) != NULL)
        {
            string = strtok(buf2, "A");
            string = strtok(NULL, "p");
            array[i]=string;
            fprintf(Temp2,"%s\n", array[i]);
            i++;
        }
        fclose(Input);
        fclose(Temp2);

        if((Temp2 = fopen("Temp2.txt", "r")) == NULL)
        {
            printf("Error Reading File\n");
            exit(0);
        }

        if((Input = fopen("Temp1.txt", "r")) == NULL)
        {
            printf("Error Reading File\n");
            exit(0);
        }

        if((Temp3 = fopen("Temp3.txt", "a+")) == NULL)
        {
            printf("Error Reading File\n");
            exit(0);
        }

        ///Printing the information in the correct order (moving digits to the end of the string as required in task
        while (fscanf(Input, "%[^\n] ", buf2) != EOF && fscanf(Temp2, "%[^\n] ", buf) != EOF)
        {
            letterH = strchr(buf2, 'H')+2;
            fprintf(Temp3,"%s%s\n",letterH,buf);
        }

        fclose(Input);
        fclose(Temp2);
        fclose(Temp3);

        char **sortArray = malloc(arraySize * sizeof(char *));
        for (i = 0; i < arraySize; i++){sortArray[i] = (char *)malloc(256);}

        if((Output = fopen("Output.txt", "a+")) == NULL)///This will be the final file
        {
            printf("Error opening output File\n");
            exit(0);
        }

        if((Input = fopen("Temp3.txt", "r")) == NULL)
        {
            printf("Error Reading File\n");
            exit(0);
        }

        i=0;
        while(fgets(buf2, 512, Input) != NULL)///saving the data to an array before sorting in Descending order
        {
            strcpy(sortArray[i], buf2);
            i++;
        }
        fclose(Input);

    //I found out about this function at the end, so I used it for the first time
    qsort (sortArray, arraySize, sizeof(char *),invertCmp);//--> called invertCmp because I want the sorting in descending order, I return -strcmp
 ////////bubble sort was my original approach but the process took ~45 minutes, which is very slow for a C program

    clock_t end = clock();////stop timer for processing data
    runTime += (double)(end - begin) / CLOCKS_PER_SEC;//calculate the difference to get elapsed time
    fprintf(Output,"%f\n",runTime);
    ////write to final Output file
	for (i = 0; i < arraySize; i++){fprintf(Output,"%s",sortArray[i]);}
    fclose(Output);
      //////delete temporal files I created///
    if (remove("Temp1.txt") != 0)printf("Could not delete file!\n");
    if (remove("Temp2.txt") != 0)printf("Could not delete file!\n");
    if (remove("Temp3.txt") != 0)printf("Could not delete file!\n");

return 0;
}

int invertCmp(const void *a, const void *b)
{
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return -strcmp(*(char **)ia, *(char **)ib);
}
