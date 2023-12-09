#include <stdio.h>
#include <stdlib.h>
typedef struct Node_t
{
    int val;
    struct Node_t *left;
    struct Node_t *right;
}Node;

void func7(Node *node, int *eax, int n){
    if (node == NULL){
        *eax = 0xffffffff;
        return;
    }
    if (node->val > n){
        func7(node->left, eax, n);
        *eax += *eax;
        return;
    }

    *eax = 0;
    if (node->val == n)
        return;

    if (node->val < n){
        func7(node->right, eax, n);
        *eax += *eax + 1;
        return;
    }
}
//               36
//       ________|________
//       |               |
//       8               50
//   ____|____       ____|____
//   |       |       |       |
//   6       22      45      107
// __|__   __|__   __|__   __|__
// |   |   |   |   |   |   |   |
// 1   7   20  35  40  47  99  1001
int main (){
    int preOrder[] = {36, 8, 6, 1, 7, 22, 20, 35, 50, 45, 40, 47, 107, 99, 1001};
    int index = 0;
    int n = 15;
    Node *root = (Node *)malloc(sizeof (Node));//小测试就不用在意什么内存泄漏了

    while (index < n){
        Node *node = (Node *)malloc(sizeof (Node));
        node->val = preOrder[index];
        node->left = NULL;
        node->right = NULL;

        if (index == 0){
            root = node;
        }
        else{
            Node *tmp = root;
            while (1){
                if (node->val < tmp->val){
                    if (tmp->left == NULL){
                        tmp->left = node;
                        break;
                    }
                    else{
                        tmp = tmp->left;
                    }
                }
                else{
                    if (tmp->right == NULL){
                        tmp->right = node;
                        break;
                    }
                    else{
                        tmp = tmp->right;
                    }
                }
            }
        }
        index++;
    }

    for (int i = 0; i < 1001; ++i){
        int eax = i;
        func7(root, &eax, i);
        if (eax == 2)
            printf("%d\n", i);
    }

    getchar();
	return 0;
}
