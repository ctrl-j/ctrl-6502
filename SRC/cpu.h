

// Instead of having separate X, Y registers:
//      For accuracy, have CURR_IND_REG and PREV_IND_REG,
//      as well as some flag keeping track of which is "X" and which is "Y".
//      That way you just swap the addresses of the two and update the ID flags,
//      instead of having to swap data