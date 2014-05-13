////////////////////////////////////////////////////////////////////////////////
// special operations

let isAsCheapAsAMove = 1 in {
  let isReMaterializable = 1 in {
    def get_global_id_i : HSAILInst<(outs GPR32:$dst), (ins i32imm:$src),
                           "workitemabsid_u32\t$dst, $src",
                           [(set (i32 GPR32:$dst),
                                 (int_HSAIL_get_global_id (i32 imm:$src)))]>;

    def get_group_id_i : HSAILInst<(outs GPR32:$dst), (ins i32imm:$src),
                        "workgroupid_u32\t$dst, $src",
                        [(set (i32 GPR32:$dst),
                              (int_HSAIL_get_group_id (i32 imm:$src)))]>;

    def get_local_id_i : HSAILInst<(outs GPR32:$dst), (ins i32imm:$src),
                         "workitemid_u32\t$dst, $src",
                         [(set (i32 GPR32:$dst),
                               (int_HSAIL_get_local_id (i32 imm:$src)))]>;

    def get_local_size_i : HSAILInst<(outs GPR32:$dst), (ins i32imm:$src),
                         "workgroupsize_u32\t$dst, $src",
                         [(set (i32 GPR32:$dst),
                               (int_HSAIL_get_local_size (i32 imm:$src)))]>;

    def get_num_groups_i : HSAILInst<(outs GPR32:$dst), (ins i32imm:$src),
                          "gridgroups_u32\t$dst, $src",
                          [(set (i32 GPR32:$dst),
                                (int_HSAIL_get_num_groups (i32 imm:$src)))]>;

    def get_global_size_i : HSAILInst<(outs GPR32:$dst), (ins i32imm:$src),
                          "gridsize_u32\t$dst, $src",
                          [(set (i32 GPR32:$dst),
                                (int_HSAIL_get_global_size (i32 imm:$src)))]>;
  }

  def get_work_dim: HSAILInst<(outs GPR32:$dst), (ins),
                    "dim_u32\t$dst",
                    [(set (i32 GPR32:$dst), 
                      (int_HSAIL_get_work_dim))]>;

  def get_workitemid_flat : HSAILInst<(outs GPR32:$dst), (ins),
                    "workitemflatid_u32\t$dst",
                    [(set (i32 GPR32:$dst), 
                      (int_HSAIL_workitemid_flat))]>;

  def do_syscall : HSAILInst<(outs GPR32:$dst), (ins i32imm:$src1, GPR32:$src2, GPR32:$src3, GPR32:$src4),
                         "syscall_u32\t$dst, $src1, $src2, $src3, $src4",
                         [(set (i32 GPR32:$dst),
                               (int_HSAIL_syscall (i32 imm:$src1), (i32 GPR32:$src2), (i32 GPR32:$src3), (i32 GPR32:$src4)))]>;

  def get_lane_id : HSAILInst<(outs GPR32:$dst), (ins),
                    "laneid_u32\t$dst",
                    [(set (i32 GPR32:$dst),
                                  (int_HSAIL_get_lane_id))]>;

  def get_dynwave_id : HSAILInst<(outs GPR32:$dst), (ins),
                       "waveid_u32\t$dst",
                       [(set (i32 GPR32:$dst),
                             (int_HSAIL_get_dynwave_id))]>;

  def get_maxdynwave_id : HSAILInst<(outs GPR32:$dst), (ins),
                          "maxwaveid_u32\t$dst",
                          [(set (i32 GPR32:$dst),
                                (int_HSAIL_get_maxdynwave_id))]>;

  def get_clock : HSAILInst<(outs GPR64:$dst), (ins),
                        "clock_u64\t$dst",
                        [(set (i64 GPR64:$dst),
                            (int_HSAIL_get_clock))]>;

  def get_cu : HSAILInst<(outs GPR32:$dst), (ins),
                        "cuid_u32\t$dst",
                        [(set (i32 GPR32:$dst),
                            (int_HSAIL_get_cu))]>;

  let isNotDuplicable = 1 in {
    def barrier : HSAILInst<(outs), (ins),
             "barrier", [(int_HSAIL_barrier)]>;

  }

  def memfence : HSAILInst<(outs), (ins), "memfence_ar_wg", [(int_HSAIL_memfence)]>;

  def memfence_global : HSAILInst<(outs), (ins), 
          "memfence_global_ar_wg", [(int_HSAIL_memfence_global)]>;

  def memfence_group : HSAILInst<(outs), (ins), 
          "memfence_group_ar_wg", [(int_HSAIL_memfence_group)]>;

}

////////////////////////////////////////////////////////////////////////////////
// SYNC OCL 2.0
		  
  def memfence_global_acq_wg : HSAILInst<(outs), (ins), 
          "memfence_global_acq_wg", [(int_HSAIL_memfence_g_acq_wg)]>;	

  def memfence_global_acq_cmp : HSAILInst<(outs), (ins), 
          "memfence_global_acq_cmp", [(int_HSAIL_memfence_g_acq_cmp)]>;	

  def memfence_global_acq_sys : HSAILInst<(outs), (ins), 
          "memfence_global_acq_sys", [(int_HSAIL_memfence_g_acq_sys)]>;	

  def memfence_global_rel_wg : HSAILInst<(outs), (ins), 
          "memfence_global_rel_wg", [(int_HSAIL_memfence_g_rel_wg)]>;	

  def memfence_global_rel_cmp : HSAILInst<(outs), (ins), 
          "memfence_global_rel_cmp", [(int_HSAIL_memfence_g_rel_cmp)]>;	

  def memfence_global_rel_sys : HSAILInst<(outs), (ins), 
          "memfence_global_rel_sys", [(int_HSAIL_memfence_g_rel_sys)]>;	

  def memfence_global_ar_wg : HSAILInst<(outs), (ins), 
          "memfence_global_ar_wg", [(int_HSAIL_memfence_g_ar_wg)]>;	

  def memfence_global_ar_cmp : HSAILInst<(outs), (ins), 
          "memfence_global_ar_cmp", [(int_HSAIL_memfence_g_ar_cmp)]>;	

  def memfence_global_ar_sys : HSAILInst<(outs), (ins), 
          "memfence_global_ar_sys", [(int_HSAIL_memfence_g_ar_sys)]>;

  def memfence_global_sc_wg : HSAILInst<(outs), (ins), 
          "memfence_global_sc_wg", [(int_HSAIL_memfence_g_sc_wg)]>;	

  def memfence_global_sc_cmp : HSAILInst<(outs), (ins), 
          "memfence_global_sc_cmp", [(int_HSAIL_memfence_g_sc_cmp)]>;	

  def memfence_global_sc_sys : HSAILInst<(outs), (ins), 
          "memfence_global_sc_sys", [(int_HSAIL_memfence_g_sc_sys)]>;			  

  def memfence_local_acq_wg : HSAILInst<(outs), (ins), 
          "memfence_group_acq_wg", [(int_HSAIL_memfence_l_acq_wg)]>;	

  def memfence_local_acq_cmp : HSAILInst<(outs), (ins), 
          "memfence_group_acq_cmp", [(int_HSAIL_memfence_l_acq_cmp)]>;	

  def memfence_local_acq_sys : HSAILInst<(outs), (ins), 
          "memfence_group_acq_sys", [(int_HSAIL_memfence_l_acq_sys)]>;	

  def memfence_local_rel_wg : HSAILInst<(outs), (ins), 
          "memfence_group_rel_wg", [(int_HSAIL_memfence_l_rel_wg)]>;	

  def memfence_local_rel_cmp : HSAILInst<(outs), (ins), 
          "memfence_group_rel_cmp", [(int_HSAIL_memfence_l_rel_cmp)]>;	

  def memfence_local_rel_sys : HSAILInst<(outs), (ins), 
          "memfence_group_rel_sys", [(int_HSAIL_memfence_l_rel_sys)]>;	

  def memfence_local_ar_wg : HSAILInst<(outs), (ins), 
          "memfence_group_ar_wg", [(int_HSAIL_memfence_l_ar_wg)]>;	

  def memfence_local_ar_cmp : HSAILInst<(outs), (ins), 
          "memfence_group_ar_cmp", [(int_HSAIL_memfence_l_ar_cmp)]>;	

  def memfence_local_ar_sys : HSAILInst<(outs), (ins), 
          "memfence_group_ar_sys", [(int_HSAIL_memfence_l_ar_sys)]>;

  def memfence_local_sc_wg : HSAILInst<(outs), (ins), 
          "memfence_group_sc_wg", [(int_HSAIL_memfence_l_sc_wg)]>;	

  def memfence_local_sc_cmp : HSAILInst<(outs), (ins), 
          "memfence_group_sc_cmp", [(int_HSAIL_memfence_l_sc_cmp)]>;	

  def memfence_local_sc_sys : HSAILInst<(outs), (ins), 
          "memfence_group_sc_sys", [(int_HSAIL_memfence_l_sc_sys)]>;

  // Cross-Lane Operations
  class CrossLaneV4<string asm>
		: HSAILInst<(outs GPR64:$dst1, GPR64:$dst2,  GPR64:$dst3,  GPR64:$dst4),
					(ins CR:$src1),
					!strconcat(asm,
							   " ( $dst1, $dst2, $dst3, $dst4 ), "
							   "$src1"), []>;

  def activelaneid_u32 :         HSAILInst<(outs GPR32:$dst), (ins),
                    "activelaneid_u32\t$dst",
                    [(set (i32 GPR32:$dst),
                          (int_HSAIL_activelaneid_u32))]>;

  def activelanecount_u32_b1 :   HSAILInst<(outs GPR32:$dst), (ins CR:$src),
                    "activelanecount_u32_b1\t$dst",
                    [(set (i32 GPR32:$dst),
                          (int_HSAIL_activelanecount_u32_b1 (i1 CR:$src)))]>;

  def activelanecount_u32_b1_i : HSAILInst<(outs GPR32:$dst), (ins i1imm:$src),
                    "activelanecount_u32_b1\t$dst",
                    [(set (i32 GPR32:$dst),
                          (int_HSAIL_activelanecount_u32_b1 (i1 imm:$src)))]>;

  defm activelaneshuffle_b1 :        CrossLaneIntrB1<
                    int_HSAIL_activelaneshuffle_b1,"activelaneshuffle_b1">;

  defm activelaneshuffle_b32 :       CrossLaneIntrB32<
                    int_HSAIL_activelaneshuffle_b32, "activelaneshuffle_b32">;

  defm activelaneshuffle_b64 :       CrossLaneIntrB64<
                    int_HSAIL_activelaneshuffle_b64, "activelaneshuffle_b64">;

  defm activelaneshuffle_width_b1 :  CrossLaneIntrB1<
                    int_HSAIL_activelaneshuffle_width_b1,
                                      "activelaneshuffle_width(WAVESIZE)_b1">;

  defm activelaneshuffle_width_b32 : CrossLaneIntrB32<
                    int_HSAIL_activelaneshuffle_width_b32,
                                      "activelaneshuffle_width(WAVESIZE)_b32">;

  defm activelaneshuffle_width_b64 : CrossLaneIntrB64<
                    int_HSAIL_activelaneshuffle_width_b64,
                                      "activelaneshuffle_width(WAVESIZE)_b64">;

  let hasWidthOperand = 1 in {
    def activelaneid_width_u32 : HSAILInst<(outs GPR32:$dst), (ins),
                      "activelaneid_width(WAVESIZE)_u32\t$dst",
                      [(set (i32 GPR32:$dst),
                            (int_HSAIL_activelaneid_width_u32))]>;

    def activelanecount_width_u32_b1 : HSAILInst<(outs GPR32:$dst),
                                                 (ins CR:$src),
                      "activelanecount_width(WAVESIZE)_u32_b1\t$dst",
                      [(set (i32 GPR32:$dst),
                            (int_HSAIL_activelanecount_width_u32_b1
                                (i1 CR:$src)))]>;
    def activelanecount_width_u32_b1_i : HSAILInst<(outs GPR32:$dst),
                                                   (ins i1imm:$src),
                      "activelanecount_width(WAVESIZE)_u32_b1\t$dst",
                      [(set (i32 GPR32:$dst),
                            (int_HSAIL_activelanecount_width_u32_b1
                                (i1 imm:$src)))]>;
  }

  let isCrossLaneInst = 1 in {
    def activelanemask_v4_b64_b1 : CrossLaneV4<"activelanemask_v4_b64_b1">;
    def activelanemask_v4_width_b64_b1 : CrossLaneV4<"activelanemask_v4_width(WAVESIZE)_b64_b1">;
  }
