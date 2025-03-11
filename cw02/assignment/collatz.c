int collatz_conjecture(int input){
    if (input % 2 == 0){
        return input/2;
    }
    else {
        return 3*input+1;
    }
}

int test_collatz_convergence(int input, int max_iter, int *steps) {
    int cnt = 1;
    steps[0] = input;
    while(input > 1 && cnt < max_iter){
        input = collatz_conjecture(input);
        steps[cnt++] = input;
    }
    if (input == 1){
        return cnt-1;
    }
    else {
        return 0;
    }
}