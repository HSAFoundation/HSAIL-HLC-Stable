#pragma OPENCL EXTENSION cl_amd_c11_atomics : enable
__kernel void linkKernel(__global int *list) {

int head;
int i;
i = get_global_id(0) + 1;
head = list[0];
if (i != get_global_size(0)) {
	do {			
			list[i] = head;
		} while (!atomic_compare_exchange_strong(&list[0], &head,i));
	}
}

__kernel void unlinkKernel(__global int *list) {
	int head;
	int i;
	int next;
	i = get_global_id(0) + 1;
	head = list[0];
	if (i != get_global_size(0)) {
		do {
			if (head == 0) return;
			next = list[list[0]];
		} while (!atomic_compare_exchange_strong (&list[0], &head, next ));
	}
}
