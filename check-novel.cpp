
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <math.h>

#if _POSIX_C_SOURCE < 200809L
#define _POSIX_C_SOURCE 200809L
#endif
#include <unistd.h>
#include "check-novel.h"
using namespace std;

// Variables needed for getopt
extern char *optarg;
extern int optind, opterr, optopt;

/* Some global variables used to quickly finish up code */
const char *optstring = "c:d:f:i:j:l:v";
char sLine[MAX_LINE+1];
unordered_set<string> stringTable;
vector<struct RSW_Boundaries> data_boundaries;
vector<struct RSW_splice *> data_splice;
const int supportPosTolerance = 5;						/// change this if necessary

/* Function Declarations */
void print_options( int v, char *del, int c, int i, int j, int length, const char *fn, const char *refflat);
int proc_options( int argc, char *argv[], int *v, char **del, int *c, int *i, int *j, int *len, char **fn);
void read_boundaries(const char *filename);
int proc_splices( FILE *ifile, int verbose, int c, int i, int j, int len, char *delim);


int main( int argc, char *argv[] )
{
	FILE *ifile;
	char *refflat_name, *ifile_name, *delimit;
	int chr_col, i_col, j_col, len_col, status, verbose;
	chr_col = i_col = j_col = len_col = -1;
	verbose = 0;

	refflat_name = NULL;
	ifile_name = NULL;
	delimit = NULL;
	if( (status = proc_options(argc, argv, &verbose, &delimit, &chr_col, &i_col, &j_col, &len_col, &ifile_name)) < 0 ) {
		exit(EXIT_FAILURE);
	}
	if( optind == argc ) {
		fprintf(stderr,"No refflat file provided.\n");
		exit(EXIT_FAILURE);
	}
	refflat_name = argv[optind];
	print_options(verbose, delimit, chr_col, i_col, j_col, len_col, ifile_name, refflat_name);

	read_boundaries(refflat_name);
	if( verbose ) {
		printf("Done reading from boundary file.\n");
	}
	if( ifile_name == NULL ) {
		ifile = fdopen(0, "r");
	}
	else {
		ifile = fopen(ifile_name, "r");
	}
	if( ifile == NULL ) {
		perror("fopen/fdopen");
		exit(EXIT_FAILURE);
	}
	if( proc_splices(ifile, verbose, chr_col, i_col, j_col, len_col, delimit) < 1 ) {
		fprintf(stderr,"Was unable to process input file, probably due to line length.\n");
	}
	fclose(ifile);
}

//int get_line(FILE *f, char s[], int maxChars)
// dat_splice: positionLarge, positionSmaller, minSmallSupport, maxLargeSupport
// minSmall, maxLarge are the ones we want
int proc_splices( FILE *ifile, int verbose, int c, int i, int j, int len, char *delim) 
{
	RSW_splice rs;
	int cnt;
	int status;
	char *token;
	char *temp_line;

	//FILE *ofile = fdopen(1, "w");
	temp_line = NULL;
	token = NULL;

	while( (status = get_line( ifile, sLine, MAX_LINE )) > 0 ) {
		if( temp_line != NULL ) {
			free(temp_line);
		}
		temp_line = strdup( sLine );
		//printf("temp_line: %s\n", temp_line);
		token = strtok(sLine, delim);
		cnt = 0;
		if( token == NULL ) {
			continue;
		}
		do {
			//printf("cnt: %d, token: %s\n", cnt, token);
			if( cnt == c ) {
				rs.chromosome = strdup(token);
			}
			else if( cnt == i ) {
				rs.minSmallSupport = atol(token);
			}
			else if( cnt == j ) {
				rs.maxLargeSupport = atol(token);
			}
			else if( cnt == len ) {
				rs.splLen = atol(token);
			}
			cnt++;
			token = strtok(NULL, delim);
		} while( token != NULL );
		//printf("2nd templine: %s\n", temp_line);
		// It goes here
		if( len == -1 ) {
			rs.splLen = abs( rs.maxLargeSupport - rs.minSmallSupport );
		}
		//printf("chr: %s, minSmall: %ld, maxLarge: %ld, splLen: %ld\n", rs.chromosome, rs.minSmallSupport, rs.maxLargeSupport, rs.splLen);
		//printf("bound_size = %d\n", data_boundaries.size());
		int a = 0;
		rs.novel = true;
		for(a=0; a < data_boundaries.size(); a++) {
			//printf("length: %d, pos1: %ld, pos2: %ld supportPosTolerance: %d\n", data_boundaries[a].length, data_boundaries[a].position1, data_boundaries[a].position2, supportPosTolerance);
			if (rs.splLen != data_boundaries[a].length) {
				continue;
			}
			if (abs(rs.minSmallSupport-data_boundaries[a].position1) <= supportPosTolerance && abs(rs.maxLargeSupport-data_boundaries[a].position2) <= supportPosTolerance) {
				break;
			}
		}
		if (a < data_boundaries.size()) { // if found in the boundaries data, not novel
			//fprintf(stderr, "Should be false\n");
			rs.novel = false;
		}
		//TODO:  this messes your output up if you use multiple delimiters

		//printf("%s,%s\n",temp_line, (rs.novel) ? "Novel" : "*" );
		//printf("%s,%s\n",temp_line, (rs.novel) ? "Novel" : "*" );
		//size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
		fwrite(temp_line, sizeof(char), strlen(temp_line) /*- 1*/, stdout);
		fwrite(delim, sizeof(char), 1, stdout);
		if( rs.novel == true ) {
			fwrite("Novel\n", sizeof(char), 6, stdout);
		} else {
			fwrite("*\n", sizeof(char), 2, stdout);
		}
		// It goes here END
		sLine[0] = '\0';
	}
	if( status < 0 ) {
		//fclose(ofile);
		return -1;
	}
	//fclose(ofile);
	return 1;
}

/*		int a;
		rs.novel = true;
		for(a=0; a < data_boundaries.size(); a++) {
			if (rs.positionLarger-rs.positionSmaller != data_boundaries[a].length) continue;
			if (abs(rs.minSmallSupport-data_boundaries[a].position1) <= supportPosTolerance &&
					abs(rs.maxLargeSupport-data_boundaries[a].position2) <= supportPosTolerance)
				break;
			}
			if (a < data_boundaries.size()) { // if found in the boundaries data, not novel
				rs.novel = false;
			}
		}
*/

/*
  Function: read_boundaries, similar to read_data but read the format of the
            refFlat file of intron/extron boundaries.
*/
void read_boundaries(const char *filename) {
  FILE * f = fopen(filename, "r");
  if (f == NULL) {printf("Error reading from file %s\n", filename); exit(0); }

  int result=1;
  while (result > 0) {
    int i; char dir;
    RSW_Boundaries * rk = new RSW_Boundaries;
    result = get_line(f, sLine, MAX_LINE);
    if (result < 0) {
      printf("Error reading data file %s, line exceeded %i characters.\n", filename, MAX_LINE);
      delete rk;
      break;
    }
    char *tempA = strtok(sLine, "\t");
    i=0;
    while (tempA != NULL) {
      string temp = tempA; temp.shrink_to_fit();
      pair<unordered_set<string>::iterator,bool> result;
      switch (i) {
      case 0:
	result = stringTable.insert(temp);
	rk->id1 = (result.first)->c_str();
	break;
      case 1:
	result = stringTable.insert(temp);
	rk->id2 = (result.first)->c_str();
	break;
      case 2:
	result = stringTable.insert(temp);
	rk->chromosome = (result.first)->c_str();
	break;
      case 3:
	rk->direction = temp[0];
	break;
      case 11:
	rk->length = atoi(temp.c_str());
	break;
      case 12:
	char *tempB = (char *) malloc(sizeof(char)* (temp.size()+1));
	strcpy(tempB,temp.c_str());
	char *temp1 = strstr(tempB, "--");
	if (temp1 == NULL) {
	  rk->position1 = rk->position2 = 0;
	}
	else {
	  temp1[0] = '\0';
	  rk->position1 = atol(tempB);
	  rk->position2 = atol(temp1+2);
	}
	free(tempB);
	break;
      }
      i++;
      tempA = strtok(NULL,"\t");
      if (tempA == NULL) break;
    }
    if (i < 13) {delete rk; break;}

    data_boundaries.push_back(*rk);
    delete rk;
  }

  fclose(f);
}



/* Arguments:
 * 	-d : delimiter <string> , default='\t' , assumes first character
 * 	-c : chromosome col <int> , default = 0
 * 	-i : start-index col <int> , default = 1
 * 	-f : input filename <string> , default = stdin
 * 	-j : end-index col <int> , default = 2
 * 	-l : length of splice <int> , default = abs(end-index)-abs(start-index)
 * 	-v : verbose
 */	
int proc_options( int argc, char *argv[], int *v, char **del, int *c, int *i, int *j, int *len, char **fn)
{
	int opt;
	if( argc < 2 ) {
		fprintf(stdout,"usage: check-novel [-cdfijv] <refflat_file>\n");
		fprintf(stdout,"  -d : delimiter <char> , default='<tab>' , only enter 1 character\n");
		fprintf(stdout,"  -c : chromosome column <int> , default = 0\n");
		fprintf(stdout,"  -i : start-index col <int> , default = 1\n");
		fprintf(stdout,"  -f : input filename <string> , default = stdin\n");
		fprintf(stdout,"  -j : end-index col <int> , default = 2\n");
		fprintf(stdout,"  -l : splice-length column <int> , default: calculated from end-index, start-index\n");
		fprintf(stdout,"  -v : verbose mode\n");
		return -1;
	}
	while( (opt = getopt(argc, argv, optstring)) != -1 ) {
		switch( opt ) {
			// delimiter, assume they follow instructions
			case 'd':
				*del = strdup(optarg); 
				break;
			// chromosome column
			case 'c':
				*c = atoi(optarg);
				break;
			// start-index column
			case 'i':
				*i = atoi(optarg);
				break;
			// end-index column
			case 'j':
				*j = atoi(optarg);
				break;
			case 'f':
				*fn = strdup(optarg);
				break;
			//verbose mode
			case 'l':
				*len = atoi(optarg);
				break;
			case 'v':
				*v = 1;	
				break;
			//unknown option
			case '?': 
				return -1;
			//option w/ missing argument
			case ':': 
				return -1;
			default: 
				fprintf(stderr,"Error: Something odd occurred in getopt.\n");
				return -1;
		}
	}
	// Set default columns for chromosome, start-index, end-index
	*c = (*c == -1) ? 0 : *c;
	*i = (*i == -1 ) ? 1 : *i;
	*j = (*j == -1 ) ? 2 : *j;
	if( *del == NULL ) {
		*del = strdup("\t");
	}
	return 1;
}

void print_options( int v, char *del, int c, int i, int j, int len, const char *fn, const char *refflat)
{
	if( v ) {
		fprintf(stdout,"Options:\n");
		fprintf(stdout,"\tdelimiter: %s\n" ,del);
		fprintf(stdout,"\tchromosome: %d\n", c);
		fprintf(stdout,"\tstart-index: %d\n", i);
		fprintf(stdout,"\tend-index: %d\n", j);
		fprintf(stdout,"\tsplice length: %d\n", len);
		fprintf(stdout,"\tverbose: %d\n", v);
		fprintf(stdout,"\tInput filename: ");
		if(fn == NULL) {
			fprintf(stdout,"stdin\n");
		} 
		else {
			fprintf(stdout,"%s\n", fn);
		}
		fprintf(stdout,"\tRefflat filename: ");
		if(refflat == NULL) {
			fprintf(stdout,"No file given.\n");
		}
		else {
			fprintf(stdout,"%s\n",refflat);
		}
	}
}
