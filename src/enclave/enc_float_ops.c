#include "enc_float_ops.h"
#include<time.h>

// extern int decrypt_status;
#if defined(TEE_TZ)
extern double pow(double x, int y);
#endif


/* this load barrier is only for arm */
#ifdef __aarch64__
	#define LOAD_BARRIER asm volatile("dsb ld" ::: "memory")
	#define STORE_BARRIER asm volatile("dsb st" ::: "memory")
#elif __x86_64
	#define LOAD_BARRIER ;
	#define STORE_BARRIER ;
#endif

int enc_float32_cmp(EncFloatCmpRequestData *req)
{
    float left,right ;
    int resp = 0;

    resp = decrypt_bytes_para((uint8_t *) &req->left, sizeof(req->left),(uint8_t*) &left, sizeof(float));

    if (resp != 0)
        return resp;
    
    resp = decrypt_bytes((uint8_t *) &req->right, sizeof(req->right),(uint8_t*) &right, sizeof(float));
    if (resp != 0)
        return resp;

    decrypt_wait((uint8_t*) &left, sizeof(left));  

    req->cmp = (left == right) ? 0 : (left < right) ? -1 : 1;
    // printf("%d, %f, %f, %d\n",req->common.reqType, left,right,req->cmp);

    return resp;
}




/* comment from PSQL code
 * There isn't any way to check for underflow of addition/subtraction
 * because numbers near the underflow value have already been rounded to
 * the point where we can't detect that the two values were originally
 * different, e.g. on x86, '1e-45'::float4 == '2e-45'::float4 ==
 * 1.4013e-45.
 * we have only 4 bytes for float4 datatype
 * we can check if the out size is less 8^4
 *
 */
int enc_float32_calc(EncFloatCalcRequestData *req)
{
    float left,right,res;
    int resp = 0;
    bool found = false, para_issued = false;

    left = float_map_find(f_map_p, &req->left, &found);
    if (!found) {
            resp = decrypt_bytes_para((uint8_t *) &req->left, sizeof(req->left), (uint8_t*) &left, sizeof(left));
            para_issued = true;
            if (resp != 0)
                return resp;
            float_map_insert(f_map_p, &req->left, &left);
    }  

    right = float_map_find(f_map_p, &req->right, &found);
    if (!found) {
        resp = decrypt_bytes((uint8_t *) &req->right, sizeof(req->right),(uint8_t*) &right, sizeof(right));
        if (resp != 0)
            return resp;
        float_map_insert(f_map_p, &req->left, &left);
    }
    if(para_issued)
        decrypt_wait((uint8_t*) &left, sizeof(left));  

    // printf("calc type %d, %f, %f, ", req->common.reqType, left, right);
    switch (req->common.reqType) /* req->common.op */
    {
    case CMD_FLOAT_PLUS:
        res = left + right;
        break;
    case CMD_FLOAT_MINUS:
        res = left - right;
        break;
    case CMD_FLOAT_MULT: 
        res = left * right;
        break;
    case CMD_FLOAT_DIV:
        res = left / right;
        break;
    case CMD_FLOAT_EXP: 
        res =  pow(left,right);
        break;
    case CMD_FLOAT_MOD:
        res = (int)left % (int)right;
        break;
    default:
        break;
    }
    // printf("%d, %f, %f, %f\n",req->common.reqType, left,right, res);

    resp = encrypt_bytes((uint8_t*) &res, sizeof(res),(uint8_t*) &req->res, sizeof(req->res));

    float_map_insert(f_map_p, (EncFloat *) &req->res, &res);

    return resp;
}



static int bulk_count = 0;
int enc_float32_bulk(EncFloatBulkRequestData *req)
{
    clock_t start, end;
    bulk_count++;

    int bulk_size = req->bulk_size;
    EncFloat *array = req->items;
    double res = 0;
    // float tmp = 0;
    float values[bulk_size];
    int count = 0, resp = 0;
    bool found = false, parallel_issued = false;
    // start = clock();
    while (count < bulk_size)
    {
        values[count] = float_map_find(f_map_p, &array[count], &found);
        if (!found) {
            if (MAX_DECRYPT_THREAD >= 15) {
                resp = decrypt_bytes_para((uint8_t *) &array[count], sizeof(EncFloat), (uint8_t*) &values[count], sizeof(float)); 
            } else {
                resp = decrypt_bytes((uint8_t *) &array[count], sizeof(EncFloat), (uint8_t*) &values[count], sizeof(float)); 
            }
            parallel_issued = true;   
            if (resp != 0)
                return resp;
        }
        count ++;
    }

    if (parallel_issued) {
        decrypt_wait(NULL, 0);
    }

    switch (req->common.reqType)
    {
    case CMD_FLOAT_SUM_BULK:
        for (int i = 0; i < bulk_size; i++) {
            res += values[i];
        }
        break;
    default:
        break;
    }
    float res1 = res;
    resp = encrypt_bytes((uint8_t*) &res1, sizeof(float),(uint8_t*) &req->res, sizeof(req->res));

    // end = clock();
    // printf("bulk function called %d times, bulk size: %d, time=%fms\n", bulk_count, bulk_size, (double)(end-start) * 1000/CLOCKS_PER_SEC);  

    return resp;
}

/**
 * @brief evaluates and calculates prefix notation expression
 * 
 * @param expr 
 * @return float 
 */
float eval_expr(char *expr, float *arr) {
    float stack[EXPR_STACK_MAX_SIZE];
    size_t i;
    int stack_top_pos = -1;
    float op1, op2;
    char c;
    memset(stack, 0, (size_t)EXPR_STACK_MAX_SIZE * sizeof(float));
    for (i = 0; i < strlen(expr); ++i) {
        c = expr[i];
        if ((int8_t)c > 0) {
            stack_top_pos++;
            stack[stack_top_pos] = arr[c - 1];
        } else {
            c = (char)-c;
            if (c == '#') {
                if (stack_top_pos < 0) {
                    printf("Missing operand!\n");
                    // exit(0);
                }
                stack[stack_top_pos] = -stack[stack_top_pos];
            } else {
                if (stack_top_pos < 1) {
                    printf("Missing operand!\n");
                    // exit(0);
                }
                op1 = (float)stack[stack_top_pos--];
                op2 = (float)stack[stack_top_pos];
                switch(c) {
                    case '+':
                        stack[stack_top_pos] = op2 + op1;
                        break;
                    case '-':
                        stack[stack_top_pos] = op2 - op1;
                        break;
                    case '*':
                        stack[stack_top_pos] = op2 * op1;
                        break;
                    case '/':
                        stack[stack_top_pos] = op2 / op1;
                        break;
                    case '%':
                        stack[stack_top_pos] = (float)((int)op2 % (int)op1);
                        break;
                    case '^':
                        stack[stack_top_pos] = (float)pow((double)op2, (double)op1);
                        break;
                    default:
                        printf("No matching operand!\n");
                        // exit(0);
                }
                if (c == '+') {
                    stack[stack_top_pos] = op1 + op2;
                }
            }
        }
    }
    return stack[stack_top_pos];
}

int enc_float32_eval_expr(EncFloatEvalExprRequestData *req)
{
    int arg_cnt = req->arg_cnt, resp = 0, i = 0;
    Str expr = req->expr;
    EncFloat *enc_arr = req->items;
    float res = 0;
    float arr[EXPR_MAX_SIZE];
    bool found = false, parallel_issued = false;
    for (i = 0; i < arg_cnt - 1; ++i) {
        arr[i] = float_map_find(f_map_p, &enc_arr[i], &found);
        if (!found) {
            resp = decrypt_bytes_para((uint8_t *) &enc_arr[i], sizeof(EncFloat), (uint8_t*) &arr[i], sizeof(float)); 
            parallel_issued = true;  
            if (resp != 0) {
                goto error;
            }
            float_map_insert(f_map_p, &enc_arr[i], &arr[i]);
        }
    }

    arr[i] = float_map_find(f_map_p, &enc_arr[i], &found);
    if (!found) {
        resp = decrypt_bytes((uint8_t *) &enc_arr[i], sizeof(EncFloat), (uint8_t*) &arr[i], sizeof(float));    
        if (resp != 0) {
            goto error;
        }
        float_map_insert(f_map_p, &enc_arr[i], &arr[i]);
    }

    if (parallel_issued) {
        decrypt_wait(NULL, 0);
    }

    res = eval_expr(expr.data, arr);
    resp = encrypt_bytes((uint8_t*) &res, sizeof(float), (uint8_t*) &req->res, sizeof(req->res));
    return resp;

error:

    if (parallel_issued) {
        decrypt_wait(NULL, 0);
    }

    return resp;
}
