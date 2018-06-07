/*
 * Copyright (C) 2011 matt mooney <mfm@muteddisk.com>
 *               2005-2007 Takahiro Hirofuchi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "usbip_windows.h"

#include "usbip_common.h"
#include "usbip_setupdi.h"
#include "usbip_stub.h"

static const char usbip_bind_usage_string[] =
	"usbip bind <args>\n"
	"    -b, --busid=<busid>    Bind usbip stub to device "
	"on <busid>\n";

void usbip_bind_usage(void)
{
	printf("usage: %s", usbip_bind_usage_string);
}

static int
walker_bind(HDEVINFO dev_info, PSP_DEVINFO_DATA pdev_info_data, devno_t devno, void *ctx)
{
	devno_t	*pdevno = (devno_t *)ctx;

	if (devno == *pdevno) {
		/* gotcha */
		if (attach_stub_driver(devno)) {
			restart_device(dev_info, pdev_info_data);
			return -100;
		}
	}
	return 0;
}

static int
bind_device(const char *busid)
{
	unsigned char	devno;
	int	ret;

	devno = get_devno_from_busid(busid);

	ret = traverse_usbdevs(walker_bind, TRUE, (void *)&devno);
	if (ret == -100) {
		info("bind device on busid %s: complete", busid);
		return 0;
	}
	err("failed to bind device");
	return 1;
}

int usbip_bind(int argc, char *argv[])
{
	static const struct option opts[] = {
		{ "busid", required_argument, NULL, 'b' },
		{ NULL,    0,                 NULL,  0  }
	};

	int opt;
	int ret = -1;

	for (;;) {
		opt = getopt_long(argc, argv, "b:", opts, NULL);

		if (opt == -1)
			break;

		switch (opt) {
		case 'b':
			ret = bind_device(optarg);
			goto out;
		default:
			goto err_out;
		}
	}

err_out:
	usbip_bind_usage();
out:
	return ret;
}
