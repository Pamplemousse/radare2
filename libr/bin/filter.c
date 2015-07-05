/* radare - LGPL - Copyright 2015 - pancake */

#include <r_bin.h>

// TODO: optimize this api:
// - bin plugins should call r_bin_filter_name() before appending

static void hashify(char *s, ut64 vaddr) {
	while (*s) {
		if (!IS_PRINTABLE(*s)) {
			if (vaddr && vaddr != UT64_MAX) {
				sprintf (s, "%"PFMT64d, vaddr);
			} else {
				ut32 hash = sdb_hash (s);
				sprintf (s, "%x", hash);
			}
			break;
		}
		s++;
	}
}

R_API void r_bin_filter_name(Sdb *db, ut64 vaddr, char *name, int maxlen) {
	ut32 hash = sdb_hash (name);
	int count = sdb_num_inc (db, sdb_fmt (0, "%x", hash), 1, 0);
	if (vaddr) {
		hashify (name, vaddr);
	}
	if (count>1) {
		int namelen = strlen (name);
		if (namelen>maxlen) name[maxlen] = 0;
		strcat (name, sdb_fmt (0,"_%d", count-1));
	//	eprintf ("Symbol '%s' dupped!\n", sym->name);
	}
}

R_API void r_bin_filter_symbols (RList *list) {
	RBinSymbol *sym;
	const int maxlen = sizeof (sym->name)-8;
	Sdb *db = sdb_new0 ();
	RListIter *iter;
	if (maxlen>0) {
		r_list_foreach (list, iter, sym) {
			r_bin_filter_name (db, sym->vaddr, sym->name, maxlen);
		}
	} else eprintf ("SymbolName is not dynamic\n");
	sdb_free (db);
}

R_API void r_bin_filter_sections (RList *list) {
	RBinSection *sec;
	const int maxlen = sizeof (sec->name)-8;
	Sdb *db = sdb_new0 ();
	RListIter *iter;
	if (maxlen>0) {
		r_list_foreach (list, iter, sec) {
			r_bin_filter_name (db, sec->vaddr, sec->name, maxlen);
		}
	} else eprintf ("SectionName is not dynamic\n");
	sdb_free (db);
}

R_API void r_bin_filter_classes (RList *list) {
	Sdb *db = sdb_new0 ();
	RListIter *iter;
	RBinClass *cls;
	r_list_foreach (list, iter, cls) {
		int namepad_len = strlen (cls->name)+10;
		char *namepad = malloc (namepad_len);
		if (namepad) {
			strcpy (namepad, cls->name);
			r_bin_filter_name (db, cls->index, namepad, namepad_len);
			//hashify (namepad, (ut64)cls->index);
			free (cls->name);
			cls->name = namepad;
		} else eprintf ("Cannot alloc %d bytes\n", namepad_len);
	}
	sdb_free (db);
}
