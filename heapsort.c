#include "pub.h"

void min_heapfy(pTZNODE* a, int i, int n)
{
	int l = i*2+1;
	int r = i*2+2;
	int smallest = i;

	if(l<n && a[i]->ctx.sprize > a[l]->ctx.sprize) smallest = l;
	if(r<n && a[smallest]->ctx.sprize > a[r]->ctx.sprize) smallest = r;

	if(smallest != i){
	    pTZNODE tmp = a[i];
	    a[i] = a[smallest];
	    a[smallest] = tmp;
		min_heapfy(a,smallest,n);
	}
}


void build_min_heap(pTZNODE* a,int n)
{
	int i;
	for(i=n/2-1; i>=0; i--){
		min_heapfy(a,i,n);
	}
}

void heap_sort(pTZNODE* a,int n)
{
	build_min_heap(a,n);
	int i;
	for(i=n-1; i>0; i--){
        pTZNODE tmp = a[i];
        a[i] = a[0];
        a[0] = tmp;
		min_heapfy(a,0,i);
	}
}
