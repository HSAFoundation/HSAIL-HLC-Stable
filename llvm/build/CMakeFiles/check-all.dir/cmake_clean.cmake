FILE(REMOVE_RECURSE
  "CMakeFiles/check-all"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/check-all.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
