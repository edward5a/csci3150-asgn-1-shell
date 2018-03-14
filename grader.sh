#!/bin/bash
############################
# CSCI3150 Shell Grader
# Usage:
# ./grader: Check All Cases
#./grader 2: Check Case 2
############################

INPUT_DIR=./testcases/input/
OUTPUT_DIR=./testcases/output/
PROG_DIR=./
EXEC=asg1-shell
EXEC_GRADE=demo-asg1
TOTAL_CASES=30

# have fun with colors and styles :)
# ref: http://misc.flogisoft.com/bash/tip_colors_and_formatting

BD="\033[1m"
BDED="\033[21m"
RED="\033[31m"
GREEN="\033[92m"
YELLOW="\033[93m"
ED="\e[0m"
diff_flags="-Z -b -B"

############################
# Main
function main {
    # Check if files exist
    if [ ! -f ${EXEC} ]; then
      echo -e "${BD}${RED}Cannot find your shell executable: asg2-shell!${ED}"
      exit -1
    fi

    if [ ! -f ${EXEC_GRADE} ]; then
      echo -e "${BD}${RED}Cannot find the demo executable: demo-asg2!${ED}"
      exit -1
    fi

    # Create output folder.
    if [ ! -d ${OUTPUT_DIR} ]; then
      mkdir -p ${OUTPUT_DIR} || { echo -e "${BD}${RED}Cannot create output directory \"${OUTPUT_DIR}\"${ED}"; exit -1; }
    fi

    rm -rf ${OUTPUT_DIR}/*

    # Provided 1 Argument: Check only the case
    if [ -n "$1" ]; then
      f=${INPUT_DIR}"in"$1

      echo ""
      echo -n "Checking Input Test Case $1 - "
      echo -e "${YELLOW}"
      cat $f | tr '\n' ',' | sed 's/.$//'
      echo -e "${ED}"

      timeout -s9 5 stdbuf -o0 ${PROG_DIR}${EXEC} < $f > ${OUTPUT_DIR}tempSTU
      timeout -s9 5 stdbuf -o0 ${PROG_DIR}${EXEC_GRADE} < $f > ${OUTPUT_DIR}tempGRADE


      diff ${diff_flags} ${OUTPUT_DIR}tempSTU ${OUTPUT_DIR}tempGRADE \
      1>/dev/null 2>/dev/null && { echo -e "${GREEN}""Test Case Passed!${ED}"; }
      diff ${diff_flags} ${OUTPUT_DIR}tempSTU ${OUTPUT_DIR}tempGRADE \
      1>/dev/null 2>/dev/null || { echo -e "${BD}${RED}""Failed case $1 \
      (top: your answer, bottom: correct answer)${ED}" \
      && diff  ${diff_flags} ${OUTPUT_DIR}tempSTU ${OUTPUT_DIR}tempGRADE; }

      exit 0

    fi

    pass=0
    total=0

    #for f in ${INPUT_DIR}*
    for i in $(seq 1 $TOTAL_CASES);
    do
      f=${INPUT_DIR}"in"$i
      stdbuf -o0 ${PROG_DIR}${EXEC} < $f > ${OUTPUT_DIR}tempSTU
      stdbuf -o0 ${PROG_DIR}${EXEC_GRADE} < $f > ${OUTPUT_DIR}tempGRADE

      diff ${diff_flags} ${OUTPUT_DIR}tempSTU ${OUTPUT_DIR}tempGRADE \
      1>/dev/null 2>/dev/null || { echo -en "${BD}${RED}""Failed case ${i} \
      (top: your answer, bottom: correct answer)${ED}" \
      && echo "" && echo -ne "${YELLOW}" && cat $f && echo -e "${ED}"\
      && diff ${diff_flags} ${OUTPUT_DIR}tempSTU ${OUTPUT_DIR}tempGRADE; } && { \
      (( pass ++ ));
      }
      ((total ++));

    done
    echo -e "${GREEN}""[Result] ${pass}/${total} test cases passed${ED}"

    if [ $((pass)) -eq $((total)) ]; then
        echo -e "${GREEN}Congrats!${ED}"
    fi


}
# echo $#
main $1
