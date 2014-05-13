FILE(REMOVE_RECURSE
  "Intrinsics.gen.tmp"
  "Intrinsics.gen"
  "CMakeFiles/intrinsics_gen"
  "Intrinsics.gen"
  "Intrinsics.gen.tmp"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/intrinsics_gen.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
