

bool fsh_unalias(pos_arguments *args, context *context){
	if (args->num_args != 1) {
		printf("Too much arguments, only alias name should be given\n")
		return false;
	char *alias = *(args->arguments);
	if (strlen(alias) == 0) {
		printf("Empty alias name given\n");
		return false;
	}	
	hashset *aliases_set = context->aliases;
	if (HashSetLookup(aliases_set, alias) == NULL){
		printf("Alias not found");
		return false;
	}
	HashSetRemove(aliases_set, &alias);
	return true;
}
