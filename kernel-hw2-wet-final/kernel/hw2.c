
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/module.h>



void aux_get_leaf(long * sum, struct task_struct * mine ){
	
  struct task_struct *my_current;
  struct list_head *iterator;
  if(list_empty(&(mine->children))) {   
    (*sum)+=mine->weight;
    return;
    }
	list_for_each(iterator,&(mine->children)){
    my_current=list_entry( iterator, struct task_struct, sibling);
    aux_get_leaf(sum,my_current);
	}
}

asmlinkage long sys_hello(void) {
 printk("Hello, World!\n");
 return 0;
}

asmlinkage long sys_set_weight(int weight) {
    if(weight < 0)
	{
	return -EINVAL;
	}
current->weight=weight;
return 0;
}


asmlinkage long sys_get_weight(void) {
return (current->weight);
}



asmlinkage long sys_get_leaf_children_sum(void) {
	long sum=0; 
	if(list_empty(&current->children)){
		//return 99;
	return -(ECHILD);
	}
	aux_get_leaf(&sum,current);
	return sum;
}



asmlinkage pid_t sys_get_heaviest_ancestor(void) {
 
	long max=current->weight;
	pid_t max_pid=current->pid;
	struct task_struct *it=current;

	while(it->pid!=1){
		if(it->weight > max)
		{
		max=it->weight;
		max_pid=it->pid;
		}
		it=it->parent;
	}

  if(it->weight > max){
	max=it->weight;
	max_pid=it->pid;
	}

return max_pid;

}
