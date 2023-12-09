int func4(int x, int y, int a){
    int ecx =  (y + (y - x < 0)) / 2;

    if (ecx > a){
        return 2 * func4(x, ecx - 1, a);
    }   
    
    int ret = 0;
    if (ecx < a){
        ret = func4(ecx + 1, y, a);
        return 2 * ret + 1;
    }

    return 0;
}