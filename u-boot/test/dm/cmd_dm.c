/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <mapmem.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/root.h>
#include <dm/util.h>

static int do_dm_dump_all(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	dm_dump_all();

	return 0;
}

static int do_dm_dump_uclass(cmd_tbl_t *cmdtp, int flag, int argc,
			     char * const argv[])
{
	dm_dump_uclass();

	return 0;
}

static cmd_tbl_t test_commands[] = {
	U_BOOT_CMD_MKENT(tree, 0, 1, do_dm_dump_all, "", ""),
	U_BOOT_CMD_MKENT(uclass, 1, 1, do_dm_dump_uclass, "", ""),
};

static int do_dm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *test_cmd;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;
	test_cmd = find_cmd_tbl(argv[1], test_commands,
				ARRAY_SIZE(test_commands));
	argc -= 2;
	argv += 2;
	if (!test_cmd || argc > test_cmd->maxargs)
		return CMD_RET_USAGE;

	ret = test_cmd->cmd(test_cmd, flag, argc, argv);

	return cmd_process_error(test_cmd, ret);
}

U_BOOT_CMD(
	dm,	3,	1,	do_dm,
	"Driver model low level access",
	"tree         Dump driver model tree ('*' = activated)\n"
	"dm uclass        Dump list of instances for each uclass"
);
