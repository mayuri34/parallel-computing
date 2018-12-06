  #include <iostream>
  #include <chrono>

  __global__ void polynomial_expansion (float* poly, int degree,
  int n, float* array) {
  	int index = blockIdx.x * blockDim.x + threadIdx.x;
	if( index < n )
  	{
		float out = 0.0;
  		float xtothepowerof = 1.0;
  		for ( int x = 0; x < degree; ++x)
  		{
  			out += xtothepowerof * poly[x];
  			xtothepowerof *= array[index];
  		}
  		array[index] = out;
  	}
  }

  int main (int argc, char* argv[]) 
  {
  	if (argc < 3) 
  	{
  		std::cerr<<"usage: "<<argv[0]<<" n degree"<<std::endl;
  		return -1;
  	}

	int n = atoi(argv[1]); 
	int degree = atoi(argv[2]);
	int nbiter = 1;

  	float* array = new float[n];
  	float* poly = new float[degree+1];
  	for (int i=0; i<n; ++i){
  		array[i] = 1.;
	}

  	for (int i=0; i<degree+1; ++i){
  		poly[i] = 1.;
	}

  	float *d_array, *d_poly;
  	std::chrono::time_point<std::chrono::system_clock> begin, end;
  	begin = std::chrono::system_clock::now();

  	cudaMalloc(&d_array, n*sizeof(float));
  	cudaMalloc(&d_poly, (degree+1)*sizeof(float));

  	cudaMemcpy(d_array, array, n*sizeof(float), cudaMemcpyHostToDevice);
  	cudaMemcpy(d_poly, poly, n*sizeof(float), cudaMemcpyHostToDevice);

  	polynomial_expansion<<<(n+255)/256, 256>>>(d_poly, degree, n, d_array);
  	cudaMemcpy(array, d_array, n*sizeof(float), cudaMemcpyDeviceToHost);

  
  	cudaFree(d_array);
  	cudaFree(d_poly);

  	cudaDeviceSynchronize();

	{
	    bool correct = true;
	    int ind;
		    for (int i=0; i< n; ++i) {
			    if (fabs(array[i]-(degree+1))>0.01) {
				    correct = false;
				    ind = i;
			    }
		    }
	    if (!correct)
	    std::cerr<<"Result is incorrect. In particular array["<<ind<<"] should be "<<degree+1<<" not "<< array[ind]<<std::endl;
	}

  	end = std::chrono::system_clock::now();
  	std::chrono::duration<double> totaltime = (end-begin)/nbiter;

  	std::cout<<n<<" "<<degree<<" "<<totaltime.count()<<std::endl;

  	delete[] array;
  	delete[] poly;

  	return 0;
  }