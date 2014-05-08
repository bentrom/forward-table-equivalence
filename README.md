forward-table-equivalence
=========================

Run an equivalence test on two forwarding tables. Output the differing IP-ranges and hops.

TO USE:
1. Compile the equivalence test
2. Run with command line arguments of table1-file table2-file output-file

Formatting for forward tables:
X.X.X.X/Y N
X is a valid ip number (0-255), Y is a valid subnet (0-31), N is the next hop router
ex:
128.192.1.0/24 4235
255.0.0.0/1 1
