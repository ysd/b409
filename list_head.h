struct list_head{
	struct list_head * prev;
	struct list_head * next;
};
static inline void list_head_init(struct list_head *lh)
{
	lh->prev = lh;
	lh->next = lh;
}
static inline void __list_add(struct list_head * n,
		struct list_head * prev,
		struct list_head * next)
{
	n->prev = prev;
	n->next = next;
	next->prev = n;
	prev->next = n;
}
static inline void list_add(struct list_head * n,struct list_head * head)
{
	__list_add(n,head,head->next);
}
static inline void list_add_tail(struct list_head * n,struct list_head * head)
{
	__list_add(n,head->prev,head);
}
