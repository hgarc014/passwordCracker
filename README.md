# passwordCracker

Generate crypt hash by running the following command

`openssl passwd -1 -salt my_salt your_pass`

compile main as
`g++ main.cc -lcrypt -pthread`

run as

`./a.out <my_salt> <hash> <max # characters> <max # threads>`
