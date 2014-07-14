extern __attribute__((pure)) uint __hsail_get_global_size(uint); 
extern __attribute__((pure)) uint __hsail_get_global_id(uint); 
extern __attribute__((pure)) uint __hsail_get_local_size(uint); 
extern __attribute__((pure)) uint __hsail_get_local_id(uint); 
extern __attribute__((pure)) uint __hsail_get_num_groups(uint); 
extern __attribute__((pure)) uint __hsail_get_group_id(uint);
extern __attribute__((pure)) uint   __hsail_get_work_dim(void);
extern __attribute__((pure)) size_t __hsail_get_global_offset(uint);

__attribute__((always_inline))
size_t get_global_offset(uint d) {
  switch(d) {
    default:
      return 0;
    case 0:
      return __hsail_get_global_offset(0);
    case 1:
      return __hsail_get_global_offset(1);
    case 2:
      return __hsail_get_global_offset(2);
  }
}

__attribute__((always_inline))
size_t get_global_id(uint d) {
  size_t id;
  switch(d) {
    default:
      id = 0;
      break;
    case 0:
      id = __hsail_get_global_id(0);
      break;
    case 1:
      id = __hsail_get_global_id(1);
      break;
    case 2:
      id = __hsail_get_global_id(2);
      break;
  }

#ifdef AMD_HSAIL_KERNARG_GLOBAL_OFFSETS
  return id + get_global_offset(d);
#else
  return id;
#endif
}

__attribute__((always_inline))
size_t get_local_id(uint d) {
  switch(d) {
    default:
      return 0;
    case 0:
      return __hsail_get_local_id(0);
    case 1:
      return __hsail_get_local_id(1);
    case 2:
      return __hsail_get_local_id(2);
  }
}

__attribute__((always_inline))
size_t get_group_id(uint d) {
  switch(d) {
    default:
      return 0;
    case 0:
      return __hsail_get_group_id(0);
    case 1:
      return __hsail_get_group_id(1);
    case 2:
      return __hsail_get_group_id(2);
  }
}

__attribute__((always_inline))
size_t get_global_size(uint d) {
  switch(d) {
    default:
      return 1;
    case 0:
      return __hsail_get_global_size(0);
    case 1:
      return __hsail_get_global_size(1);
    case 2:
      return __hsail_get_global_size(2);
  }
}

__attribute__((always_inline))
size_t get_local_size(uint d) {
  switch(d) {
    default:
      return 1;
    case 0:
      return __hsail_get_local_size(0);
    case 1:
      return __hsail_get_local_size(1);
    case 2:
      return __hsail_get_local_size(2);
  }
}

__attribute__((always_inline))
size_t get_enqueue_local_size(uint d) {
  switch(d) {
    default:
      return 1;
    case 0:
      return __hsail_get_local_size(0);
    case 1:
      return __hsail_get_local_size(1);
    case 2:
      return __hsail_get_local_size(2);
  }
}

__attribute__((always_inline))
size_t get_num_groups(uint d) {
  switch(d) {
    default:
      return 1;
    case 0:
      return __hsail_get_num_groups(0);
    case 1:
      return __hsail_get_num_groups(1);
    case 2:
      return __hsail_get_num_groups(2);
  }
}

__attribute__((always_inline))
uint get_work_dim() {
  return __hsail_get_work_dim();
}

