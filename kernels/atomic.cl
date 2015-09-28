

// Global atomic, relatively slow
kernel void AtomicSum(global int* sum, global int* a){
	const int i = get_global_id(0);

	atomic_add(sum, a[i]);

	
	if (i == 0)
		printf("Printf end\n");
}

//Moving the sum function to a local variable, afterwards add it
//to the global in atomic, increasing the speed by alot.
kernel void AtomicSum2(global int* sum, global int* a){
	const int i = get_global_id(0);

	//Good way to initiate?
	local int tmpSum[1];
	if (get_local_id(0) == 0){
		tmpSum[0] = 0;
	}
	//Ensure tmpSum is initialized
	barrier(CLK_LOCAL_MEM_FENCE);

	atomic_add(&tmpSum[0], a[i]);
	
	//Ensure that all threads where executed
	barrier(CLK_LOCAL_MEM_FENCE);

	if (get_local_id(0) == (get_local_size(0) - 1)){
		atomic_add(sum, tmpSum[0]);
	}

	if(i == 0)
		printf("Printf end\n");
}

//Same as AtomicSum2 but then with 4 local variables
kernel void AtomicSum3(global int* sum, global int* a){
	const int i = get_global_id(0);

	local int tmpSum[4];
	if (get_local_id(0)<4){
		tmpSum[get_local_id(0)] = 0;
	}
	//Ensure tmpSum is initialized
	barrier(CLK_LOCAL_MEM_FENCE);

	atomic_add(&tmpSum[get_global_id(0) % 4], a[i]);

	//Ensure that all threads where executed
	barrier(CLK_LOCAL_MEM_FENCE);
	if (get_local_id(0) == (get_local_size(0) - 1)){
		atomic_add(sum, tmpSum[0] + tmpSum[1] + tmpSum[2] + tmpSum[3]);
	}

	if (i == 0)
		printf("Printf end\n");
}