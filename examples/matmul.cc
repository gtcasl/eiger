/*!
  \file matmul.h
  \author Eric Anger <eanger@gatech.edu>
  \date Sept 20, 2012
  \brief Example matrix multiplication application and interface with Eiger
*/

#include <iomanip>
#include <iostream>
#include <time.h>
#include <stdlib.h>

/*** Include file for all eiger functionality ***/
#include <eiger.h>

typedef struct timespec timespec;

/*
 * Calculates the elapsed time between two timespec objects
 */
float get_elapsed(timespec *start, timespec *end){
	float res = 0;
	if((end->tv_nsec-start->tv_nsec) < 0){
		res += end->tv_sec - start->tv_sec - 1;
		res += (1000000000 + end->tv_nsec - start->tv_nsec) / 1000000000.0f;
	}
	else{
		res += end->tv_sec - start->tv_sec;
		res += (end->tv_nsec - start->tv_nsec) / 1000000000.0f;
	}
  return res;
}

/*
 * Main application for performing matrix multiplication.
 * This will loop over progressively larger matrices then 
 * send the appropriate information to the Eiger internal 
 * database.
 */
int main(){

	int n; 
	int max = 500; // largest matrix
	float **A, **B, **C;
	int i,j,k;
	float sum;
	timespec start, end;
	float elapsed;

	srand(0);

	/*** Begin initialization ***/
	A = new float*[max];
	B = new float*[max];
	C = new float*[max];
	for(i=0; i<max; i++){
		A[i] = new float[max];
		B[i] = new float[max];
		C[i] = new float[max];
	}
	if(A == NULL || B == NULL || C == NULL){
		std::cerr << "Error: could not allocate memory" << std::endl;
		exit(0);
	}

	for(i=0; i<max; i++){
		for(j=0; j<max; j++){
			A[i][j] = (float)rand() / (float)RAND_MAX;
			B[i][j] = (float)rand() / (float)RAND_MAX;
		}
	}
	/*** End initialization ***/

	/*** Connect to the Eiger database ***/
	std::string location, name, user, pw, dc_name;
	std::cout << "Enter database location: ";
	std::cin >> location;
	std::cout << "Enter database name: ";
	std::cin >> name;
	std::cout << "Enter database username: ";
	std::cin >> user;
	std::cout << "Enter database password: ";
	std::cin >> pw;
  std::cout << "Enter data collection name: ";
  std::cin >> dc_name;

	eiger::Connect(location, name, user, pw);
	
	/*** Setup all Eiger objects relating to this model ***/
	eiger::DataCollection dc(dc_name, "test dc");
	dc.commit();
	eiger::Machine machine("test", "test machine");
	machine.commit();
	eiger::Application application("test", "test application");
	application.commit();
	eiger::Dataset dataset(application.getID(), "test", "test dataset", "test dataset url");
	dataset.commit();
	eiger::PropertiesID propID;

	/*** Create both the training metrics and the result metrics ***/
	eiger::Metric size(eiger::NONDETERMINISTIC, "size", "matrix size");
	size.commit();
	eiger::Metric runtime(eiger::RESULT, "runtime", "runtime of this matmul");
	runtime.commit();

	/*** Loop over different size matrices. There is one trial per matrix size ***/
	std::cout << "Beginning execution..." << std::setprecision(5) << std::endl;
	for(n=10; n <= max; n+=10){

		/*** Actual matrix calculation ***/
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
		for(i=0; i<n; i++){
			for(j=0; j<n; j++){
				sum = 0.0;
				for(k=0; k<n; k++){
					sum += A[j][k] * B[k][i];
				}
				C[j][i] = sum;
			}
		}
		
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
		elapsed = get_elapsed(&start, &end);
		std::cout << n << "x" << n << "\t: " << elapsed << " seconds" << std::endl;
		/*** End matrix calculation ***/

		/*** Create a new trial for this particular matrix size ***/
		eiger::Trial trial(dc.getID(), 
						   machine.getID(), 
						   application.getID(), 
						   dataset.getID(), 
						   propID);
		trial.commit();

		/*** There is only one execution per trial, as this is the
		   only data acquisition pass we will make ***/
		eiger::Execution execution(trial.getID(), machine.getID());
		execution.commit();

		/*** Actually store the values pertaining to this execution ***/
		eiger::NondeterministicMetric sm(execution.getID(), size.getID(), n);
		sm.commit();
		eiger::NondeterministicMetric rm(execution.getID(), runtime.getID(), elapsed);
		rm.commit();

	}

	std::cout << "End of execution, cleaning up." << std::endl;

	/*** Begin clean up ***/
	eiger::Disconnect();

	delete A;
	delete B;
	delete C;
	/*** End clean up ***/
	return 0;
}

