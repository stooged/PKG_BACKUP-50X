#include "ps4.h"
#include "patch.h"

#define INI_FILE "PKG_BACKUP.ini"

int nthread_run;
char notify_buf[1024];
char ini_file_path[256];
char usb_mount_path[256];
char backup_path[256];
int  xfer_pct;
long xfer_cnt;
char *cfile;
int tmpcnt;
int isxfer;


void makeini()
{
    if (!file_exists(ini_file_path)) 
    {
    int ini = open(ini_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    char *buffer;
    buffer ="To backup official pkg files uncomment the line below.\r\n//BACKUP_RETAIL\r\n\r\nTo backup updates to the usb hdd uncomment the line below.\r\n//BACKUP_UPDATES\r\n\r\nTo backup DLC to the usb hdd uncomment the line below.\r\n//BACKUP_DLC\r\n\r\nTo use this list as a list of games you want to backup not ignore then uncomment the line below.\r\n//BACKUP_LIST\r\n\r\nExample ignore or backup usage.\r\n\r\nCUSAXXXX1\r\nCUSAXXXX2\r\nCUSAXXXX3\r\n";
    write(ini, buffer, strlen(buffer));
    close(ini);
    }
}


char *getContentID(char* pkgFile)
{
        char buffer[37];
        char *retval = malloc(sizeof(char)*37);
        int pfile = open(pkgFile, O_RDONLY, 0);
        lseek (pfile, 64, SEEK_SET);
        read(pfile, buffer, sizeof(buffer));
        close(pfile);
        strcpy(retval, buffer);
        return retval;
}


char *getPkgName(char* sourcefile)
{
   char *retval = malloc(sizeof(char)*256);
   char *jfile = replace_str(sourcefile, ".pkg", ".json");
   if (file_exists(jfile)) 
   {
      int cfile = open(jfile, O_RDONLY, 0);
      char *idata = read_string(cfile);
      close(cfile);
      char *ret;
      ret = strstr(idata, "\"url\":\"");
      if (ret != NULL)
      {
         int bcnt = 0;
         char **buf = NULL;
         bcnt = split_string(ret, '/', &buf);
         split_string(buf[bcnt - 1], '"', &buf);
         if (strlen(buf[0]) > 0)
         {
            buf[0] = replace_str(buf[0], ".pkg", "");
            strcpy(retval, buf[0]);
            return retval;
         }
      }   
   }
        char *cid = getContentID(sourcefile);
        strcpy(retval, cid);
        free(cid);
        return retval;       
}


int isinlist(char *sourcefile)
{
        if (file_exists(ini_file_path)) 
        {
            int cfile = open(ini_file_path, O_RDONLY, 0);
            char *idata = read_string(cfile);
            close(cfile);
            if (strlen(idata) != 0)
            {
			    char *tmpstr;
			    char *srcfile;
			    srcfile = replace_str(sourcefile, "/mnt/ext0", "");
                if (strstr(srcfile, "/user/app/") != NULL)
                {	
                tmpstr = replace_str(srcfile, "/user/app/", "");
                tmpstr = replace_str(tmpstr, "/app.pkg", "");
                }
                else if (strstr(srcfile, "/user/patch/") != NULL)
                {
                tmpstr = replace_str(srcfile, "/user/patch/", "");
                tmpstr = replace_str(tmpstr, "/patch.pkg", "");
                }
                else
                {
                tmpstr = replace_str(srcfile, "/user/addcont/", "");	
                char **buf = NULL;
                split_string(tmpstr,'/',&buf);
                tmpstr = buf[0];
                }
                if(strstr(idata, tmpstr) != NULL) 
                {
                   return 1;
                }
             return 0;
             }
        return 0;
        }
        else
        {
             return 0;
        }
}


int isbacklist()
{
        if (file_exists(ini_file_path)) 
        {
            int cfile = open(ini_file_path, O_RDONLY, 0);
            char *idata = read_string(cfile);
            close(cfile);
            if (strlen(idata) != 0)
            {
                if(strstr(idata, "//BACKUP_LIST") != NULL) 
                {
                   return 0;
                }
                else if(strstr(idata, "BACKUP_LIST") != NULL) 
                {
                   return 1;
                }
             return 0;
             }
        return 0;
        }
        else
        {
             return 0;
        }
}


int isupdates()
{
        if (file_exists(ini_file_path)) 
        {
            int cfile = open(ini_file_path, O_RDONLY, 0);
            char *idata = read_string(cfile);
            close(cfile);
            if (strlen(idata) != 0)
            {
                if(strstr(idata, "//BACKUP_UPDATES") != NULL) 
                {
                   return 0;
                }
                else if(strstr(idata, "BACKUP_UPDATES") != NULL) 
                {
                   return 1;
                }
             return 0;
             }
        return 0;
        }
        else
        {
             return 0;
        }
}


int isdlc()
{
        if (file_exists(ini_file_path)) 
        {
            int cfile = open(ini_file_path, O_RDONLY, 0);
            char *idata = read_string(cfile);
            close(cfile);
            if (strlen(idata) != 0)
            {
                if(strstr(idata, "//BACKUP_DLC") != NULL) 
                {
                   return 0;
                }
                else if(strstr(idata, "BACKUP_DLC") != NULL) 
                {
                   return 1;
                }
             return 0;
             }
        return 0;
        }
        else
        {
             return 0;
        }
}


int isretpkg()
{
        if (file_exists(ini_file_path)) 
        {
            int cfile = open(ini_file_path, O_RDONLY, 0);
            char *idata = read_string(cfile);
            close(cfile);
            if (strlen(idata) != 0)
            {
                if(strstr(idata, "//BACKUP_RETAIL") != NULL) 
                {
                   return 0;
                }
                else if(strstr(idata, "BACKUP_RETAIL") != NULL) 
                {
                   return 1;
                }
             return 0;
             }
        return 0;
        }
        else
        {
             return 0;
        }
}


void copyFile(char *sourcefile, char* destfile)
{
    int src = open(sourcefile, O_RDONLY, 0);
    if (src != -1)
    {
        int out = open(destfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (out != -1)
        {
            cfile = sourcefile;
            isxfer = 1;
            size_t bytes, bytes_size, bytes_copied = 0;
            char *buffer = malloc(65536);
            if (buffer != NULL)
            {
                lseek(src, 0L, SEEK_END);
                bytes_size = lseek(src, 0, SEEK_CUR);
                lseek(src, 0L, SEEK_SET);
                while (0 < (bytes = read(src, buffer, 65536))) {
                    write(out, buffer, bytes);
                    bytes_copied += bytes;
                    if (bytes_copied > bytes_size) bytes_copied = bytes_size;
                   xfer_pct = bytes_copied * 100 / bytes_size;
                   xfer_cnt += bytes;
                }
                free(buffer);
            }
            close(out);
            isxfer = 0;
            xfer_pct = 0;
            xfer_cnt = 0;
        }
        else {
        }
        close(src);
    }
    else {
    }
}


void copypkg(char *sourcepath, char* destpath)
{       
    if (isfpkg(sourcepath) == 0 || strstr(sourcepath, "ac.pkg") != NULL || isretpkg()) 
	{
		char cmsg[1024];
		char dstfile[256];
		char *ndestpath;
		char *pknm = getPkgName(sourcepath);
		sprintf(dstfile, "%s.pkg", pknm);
		free(pknm);
		if(strstr(sourcepath, "app.pkg") != NULL)
		{
			ndestpath = replace_str(destpath, "app.pkg", dstfile);
		}
		else if(strstr(sourcepath, "patch.pkg") != NULL)
		{
			ndestpath = replace_str(destpath, "patch.pkg", dstfile);
		}
		else
		{
			ndestpath = replace_str(destpath, "ac.pkg", dstfile);
		}
		if (!file_exists(ndestpath)) 
		{
			sprintf(cmsg, "%s\n%s", "Processing:" , sourcepath);
			systemMessage(cmsg);
			copyFile(sourcepath, ndestpath);
		}
		else
		{
			if (!file_compare(sourcepath, ndestpath))
			{
				sprintf(cmsg, "%s\n%s\nOverwriting as pkg files are mismatched", "Found pkg at " , ndestpath);
				systemMessage(cmsg);
				copyFile(sourcepath, ndestpath);
			} 
		}
    }
}


void copyDir(char *sourcedir, char* destdir)
{
    DIR *dir;
    struct dirent *dp;
    struct stat info;
    char src_path[1024], dst_path[1024];
    dir = opendir(sourcedir);
    if (!dir)
        return;
    while ((dp = readdir(dir)) != NULL)
    {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
        {}
        else
        {
            sprintf(src_path, "%s/%s", sourcedir, dp->d_name);	
            if (!stat(src_path, &info))
            {
                if (S_ISDIR(info.st_mode))
                {
                  copyDir(src_path, dst_path);
                }
                else
                if (S_ISREG(info.st_mode))
                {
					if(strstr(src_path, "app.pkg") != NULL || strstr(src_path, "patch.pkg") != NULL || strstr(src_path, "ac.pkg") != NULL) 
					{
						if(strstr(src_path, "app.pkg"))
						{
							sprintf(dst_path, "%s/APPS/%s", backup_path  , dp->d_name);
						}
						else if(strstr(src_path, "patch.pkg"))
						{
							sprintf(dst_path, "%s/UPDATES/%s", backup_path  , dp->d_name);
						}
						else if(strstr(src_path, "ac.pkg"))
						{
							sprintf(dst_path, "%s/DLC/%s", backup_path  , dp->d_name);	
						}
						if (isbacklist() )
						{
							if (isinlist(src_path) )
							{
								copypkg(src_path, dst_path);
							}
						}
						else
						{
							if (!isinlist(src_path) )
							{
								copypkg(src_path, dst_path);
							}
						}
					}
                }
            }
        }
    }
    closedir(dir);
}


void *nthread_func(void *arg)
{
	time_t t1, t2;
	t1 = 0;
	while (nthread_run)
	{
		if (isxfer)
		{
			t2 = time(NULL);
			if ((t2 - t1) >= 15)
			{
				t1 = t2;
				if (tmpcnt >= 1048576)
				{
				   sprintf(notify_buf, "Copying: %s\n\n%u%% completed\nSpeed: %u MB/s", cfile , xfer_pct, tmpcnt / 1048576);
				}
				else if (tmpcnt >= 1024)
				{
				   sprintf(notify_buf, "Copying: %s\n\n%u%% completed\nSpeed: %u KB/s", cfile , xfer_pct, tmpcnt / 1024);
				}
				else
				{
				   sprintf(notify_buf, "Copying: %s\n\n%u%% completed\nSpeed: %u B/s", cfile , xfer_pct, tmpcnt);
				}			
				systemMessage(notify_buf);
			}
		}
		else t1 = 0;
		sceKernelSleep(1);
	}
	return NULL;
}


void *sthread_func(void *arg)
{
	while (nthread_run)
	{
           if (isxfer)
           {
              tmpcnt = xfer_cnt;
              xfer_cnt = 0;
           }
          sceKernelSleep(1);
	}
	return NULL;
}


char* getusbpath()
{
    int usbdir;
    char tmppath[64];
    char bakpath[64];
	char inipath[64];
    char tmpusb[64];
    tmpusb[0] = '\0';
    char *retval;
    for (int x = 0; x <= 7; x++)
    {
		sprintf(tmppath, "/mnt/usb%i/.dirtest", x);
		usbdir = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
		if (usbdir != -1)
		{
			close(usbdir);
			unlink(tmppath);
			sprintf(tmpusb, "/mnt/usb%i", x);
			sprintf(bakpath, "%s/PKG_BACKUP/", tmpusb);
			sprintf(inipath, "%s/%s", tmpusb, INI_FILE);
			if (dir_exists(bakpath) || file_exists(inipath))
			{
				retval = malloc (sizeof (char) * 10);
				strcpy(retval, tmpusb);
				return retval;
			}
		}
	}
    if (tmpusb[0] != '\0')
	{
		retval = malloc (sizeof (char) * 10);
		strcpy(retval, tmpusb);
		return retval;
	}
	return NULL;
}


int _main(struct thread *td) {
	initKernel();
	initLibc();
	initPthread();
	DIR *dir;
	dir = opendir("/user/app");
	if (!dir)
	{
		syscall(11,patcher,td);
	}
	else
	{
		closedir(dir);
	}
	initSysUtil();
	xfer_cnt = 0;
	isxfer = 0;
	nthread_run = 1;
	int hasext = 0;
	ScePthread nthread;
	scePthreadCreate(&nthread, NULL, nthread_func, NULL, "nthread");
	ScePthread sthread;
	scePthreadCreate(&sthread, NULL, sthread_func, NULL, "sthread");
	char* usb_mnt_path = getusbpath();
	if (usb_mnt_path != NULL)
	{
		systemMessage("Backing up installed fpkg files to usb\n\nThis may take a while");
		sceKernelSleep(5);
		sprintf(usb_mount_path, "%s", usb_mnt_path);
		free(usb_mnt_path);
		char tmppath[256];
		sprintf(backup_path, "%s/PKG_BACKUP", usb_mount_path);
		if (!dir_exists(backup_path)) 
		{
			mkdir(backup_path, 0777);
		}
		sprintf(ini_file_path, "%s/%s", usb_mount_path, INI_FILE);
		if (!file_exists(ini_file_path))
		{
			makeini();
		}
		DIR *extdrive;
	    extdrive = opendir("/mnt/ext0/user/app");
	    if (extdrive)
		{
			closedir(extdrive);
			hasext = 1;
		}
		sprintf(tmppath, "%s/PKG_BACKUP/APPS", usb_mount_path);
		if (!dir_exists(tmppath)) 
		{
			mkdir(tmppath, 0777);
		}
		systemMessage("Backing up apps from system drive to USB");
		copyDir("/user/app",backup_path);
		if (hasext == 1)
		{
			systemMessage("Backing up apps from ext drive to USB");
			copyDir("/mnt/ext0/user/app",backup_path);
		}
		if (isupdates())
		{
			sprintf(tmppath, "%s/PKG_BACKUP/UPDATES", usb_mount_path);
			if (!dir_exists(tmppath)) 
			{
				mkdir(tmppath, 0777);
			}
			systemMessage("Backing up updates from system drive to USB");
			copyDir("/user/patch",backup_path);
			if (hasext == 1)
			{
				systemMessage("Backing up updates from ext drive to USB");
				copyDir("/mnt/ext0/user/patch",backup_path);
			}	
		}
		if (isdlc())
		{
			sprintf(tmppath, "%s/PKG_BACKUP/DLC", usb_mount_path);
			if (!dir_exists(tmppath)) 
			{
				mkdir(tmppath, 0777);
			}
			systemMessage("Backing up dlc from system drive to USB");
			copyDir("/user/addcont",backup_path); 
			if (hasext == 1)
			{
				systemMessage("Backing up dlc from ext drive to USB");
				copyDir("/mnt/ext0/user/addcont",backup_path); 
			}	
		}
		systemMessage("Complete.");
	}
	else
	{
		systemMessage("No usb hdd found.\n\nYou must use a exFat formatted usb hdd");	
	}
	nthread_run = 0;
	return 0;
}
