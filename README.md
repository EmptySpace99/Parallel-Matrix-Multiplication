# Parallel Matrix Multiplication

### How it works?
A very simple way of parallelizing matrix calculations is based on decompositions into
blocks of the matrices on which one operates.

Example: we consider a matrix A (M x N) and a vector V (N x 1).

In the row decomposition matrix A is divided into T blocks of R rows (M = TxR)

Each block RxN can then be multiplied by V obtaining a new vector R x 1

By concatenating the resulting T vectors (see figure below) we obtain the vector A x V (M x 1).

![Cattura](https://user-images.githubusercontent.com/62540354/116433645-003d5d80-a84a-11eb-920a-49a0015d02da.PNG)

### What it does?

More precisely, the program performs the multiplication C * (A * B) between matrices of
float where:

• A is an MxN matrix,

• B is an NxP matrix,

• C is a PxM matrix

To take advantage of multithreading I used row decomposition to calculate both R = A * B
that C * R (the result is a PxP matrix).
