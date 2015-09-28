__kernel void BRANCH(__global int* neg)
{
	const int i = get_global_id(0);
	if (neg[i] == -1) {
		for (int i = 0; i < 100000; i++) neg[i] = neg[i];
	}
	else {
		for (int i = 0; i < 100000; i++) neg[i] = neg[i];
	}
}

__kernel void NOBRANCH(__global int* neg)
{
	const int i = get_global_id(0);
	for (int i = 0; i < 100000; i++) neg[i] = neg[i];
}