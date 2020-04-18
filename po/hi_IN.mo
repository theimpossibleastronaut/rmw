��    K      t  e   �      `  '   a  �   �  �    K  �  �   *
  �   �
  Y   e  
   �     �     �  @   �  �   1  1   �  2   �     $  Q   C     �  
   �  /   �  $   �        $   (      M  3   n  H   �  .   �          ,     >     J  $   `  ,   �  =   �     �  N        U  &   q  &   �  '   �  D   �     ,     ;     Q  %   h  &   �  3   �     �  .   	      8  �  Y     M     V     g     }  +   �     �  �   �  *   z  3   �     �     �  =        B  a   S  .   �  !   �            .   +     Z     l     �  %   �  3   �  H  �  -   >  �   l  4    ~  I  �   �  �   M  L         d      q      �   *   �   �   �   8   U!  7   �!  ?   �!  S   "  %   Z"     �"  8   �"  -   �"  (   �"  ,   #     I#  4   i#  `   �#  /   �#     /$     E$     ^$     n$     �$  .   �$  @   �$     %  C   2%     v%  #   �%  #   �%  :   �%  C   &     Y&     n&     �&  2   �&  #   �&  ,   �&  -   '  0   K'     |'  �  �'     �*     �*     �*     �*  .   �*     �*  �   +  :   �+  ;   �+      ,     ?,  ;   U,     �,  S   �,  ,   �,     &-     @-     T-  $   e-     �-     �-     �-  "   �-  4   �-     H       '   F             0      ;   C   2   6         8             %           (   :         
   E   B   	   >   D                 !   ,          +       *      &   7   4          @   K       I   $       .   3           /          J       1                       -          "             )       A   #   G          ?                              <   9       5   =        
# A folder can use the $UID variable.
 
# How many days should files be allowed to stay in the waste folders before
# they are permanently deleted
#
# use '0' to disable purging
#
 
# If you would like this to be your primary trash folder (which usually means
# that it will be the same as your Desktop Trash folder) be sure it precedes
# any other WASTE folders listed in the config file
#
# If you want it checked for files that need purging, simply uncomment
# The line below. Files you move with rmw will go to the folder above by
# default.
#
# Note to OSX and Windows users: sending files to 'Desktop' trash
# doesn't work yet
#
 
# Removable media: If a folder has ',removable' appended to it, rmw
# will not try to create it; it must be initially created manually. If
# the folder exists when rmw is run, it will be used; if not, it will be
# skipped Once you create "example_waste", rmw will automatically create
# example_waste/info and example_waste/files
 
# purge is allowed to run without the '-f' option. If you'd rather
# require the use of '-f', you may uncomment the line below.
#
   

  	===] Restoring [===

  -z, --restore <wildcard filename(s) pattern>
  -s, --select              select files from list to restore
  -u, --undo-last           undo last ReMove
   

Visit the rmw home page for more help, and information about
how to obtain support -    :error:    :return code: %d -- %s
  :warning:   <--> Displaying part of the string that caused the error <-->

 # NOTE: If two WASTE folders are on the same file system, rmw will move files
# to the first WASTE folder listed, ignoring the second one.
#
 %d directories skipped (RMDIR_MAX_DEPTH reached)
 %d directories skipped (contains read-only files)
 %d file purged %d files purged %d file was removed to the waste folder %d files were removed to the waste folder %s freed %s freed 'q' - quit (%d directory deleted) (%d directories deleted) (%d file deleted) (%d files deleted) (check owner/write permissions)
 -i / --interactive: not implemented
 -r / --recurse: not implemented
 <CURSOR-RIGHT / CURSOR-LEFT> - switch waste folders <SPACE> - select or unselect an item. / <ENTER> - restore selected items == contains %d file == == contains %d files == Action cancelled. Continue? (y/n):  Created %s
 Created directory %s
 Creating default configuration file: Directory not purged - still contains files
 Duplicate filename at destination - appending time string...
 File not found: '%s'
 Insufficient command line arguments given;
Enter '%s -h' for more information
 Invalid WASTE option: '%s'
 Maximum depth of %u reached, skipping
 No suitable filesystem found for "%s"
 Purging all files in waste folders ...
 Purging files based on number of days in the waste folders (%u) ...
 Reading %s...
 Report bugs to <%s>.
 Restore() returned %d
 Searching using only the basename...
 The Easter Bunny says, "Hello, world." The contents of all waste folders will be deleted - Unable to continue. Exiting...
 Unable to read or write a configuration file.
 Unknown or invalid option: '%s'
 Usage: rmw [OPTION]... FILE...
ReMove the FILE(s) to a WASTE directory listed in configuration file

   or: rmw -s
   or: rmw -u
   or: rmw -z FILE...
Restore FILE(s) from a WASTE directory

  -h, --help
  -c, --config filename     use an alternate configuration
  -l, --list                list waste directories
  -g, --purge               run purge even if it's been run today
  -o, --orphaned            check for orphaned files (maintenance)
  -f, --force               allow rmw to purge files in the background
  -e, --empty               completely empty (purge) all waste folders
  -v, --verbose             increase output messages
  -w, --warranty            display warranty
  -V, --version             display version and license information
 attached config file: %s
 dry-run mode enabled. failed to remove %s
 format of .trashinfo `file %s` is incorrect function: <%s> no usable WASTE folder could be found
Please check your configuration file and permissions
If you need further help, or to report a possible bug,
visit the rmw web site at
 purge has been skipped: use -f or --force
 purging is disabled ('purge_after' is set to '0')

 realpath() returned an error.
 removable,  rmw: %s(): buffer too small (got %d, needed a minimum of %d)
 search complete
 unable to create config and data directory
Please check your configuration file and permissions

 while attempting to allocate memory -- %s:L%d
 while changing permissions of %s
 while closing %s
 while creating %s
 while getting the path to your home directory
 while opening %s
 while opening %s -- %s:L%d
 while removing %s
 while removing .trashinfo file: '%s'
 while trying to move (rename)
  %s -> %s -- %s:L%d
 Project-Id-Version: rmw 0.7.04
Report-Msgid-Bugs-To: andy400-dev@yahoo.com
PO-Revision-Date: 2020-04-18 12:26+0530
Language-Team: 
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=2; plural=(n==0 || n==1);
X-Generator: Poedit 2.3
Last-Translator: 
Language: hi_IN
 
# folder $UID variable ka istemal karta hai
  
# Kitne dino tak failonko ko pehle apshist folder may rehne ki anumati di jani chahiye?
# Yeh sthayi roop se hata di jati hai

# mitane ke liye '0' ka upyog karen
#
 
# yadi aap chahte hai ki yeh aapka prathamik kachra folder ho(jiska aamtor par matlab hota hai
# ki yeh aapko desktop trash folder ke saamna hoga) sunischit karen ki yeh pehle
# config file may suchibandh bhi anya apshist folder
#
# yadi aap chahte hai k yeh unn failon ke liye jaanch kiya jaye jinhe mitane ki abyasakta hai, toh bas tipanni na kare 
# niche ki rekha| failen jo aap rmw hatayega to woh upar wale folder ko jayega   
# default।
#
# OSX aur Windows upyogi hai to dhyan de: 'Desktop' kachre keliye folder bhejna  
# abhi tak kaam nahi karta hai
#
 # hatane yogya media: agar ek folder hai hatane yogya, yeh karne ke liye sanlagn hai, rmw
# isse banana ki kosish nahi karega, yeh suru se manually roop se banaya jana chahiye, yadi
# woh folder rehte rmw ko chalaya toh woh folder upyog kiya jayega, yadi nahi hoga
# ek baar jab 'example_waste" banate hai, toh woh khudh ban jayega
# example_waste/info aur example_waste/files may

  
# shudh 'f' bikalp ke bina chalane ki anumati hai yadi | agar appko
# 'f' ka prayog karna hai to niche Rekha par comment karen 
#
   
  
  	 ===] Bahal [===

  -z, --bahal <wildcard filename(s) pattern>
  -s ,--bahal karneke liye suchi se chuninda         failon ka chayan karen
  -u, --purbith-pichle          purbit pichle ReMove
 adhik madad aur jankari ke liye rmw homepage jaaye
aur samtharna prapt karen    :galati:    :return code: %d -- %s
  :chetawani:   <---> galati ke karan bana string <--->

 # Dhyan de :Agar do aphsist folder ek file system may hai, rmw unko hata dega
# pehle aphsist folder ko likhke, dusre ko nazarandaz kiya jayega
 % directories ko chodo gaya hai(RMDIR_MAX_DEPTH hogaya)
 %d directories chodi gayi hai (sirf padhne wali files)
 %d failen sudhikaran ki gayi hai %d file sudhikaran ki gayi hai %d failen hatayi gayi hai waste folder ko %d files were removed to the waste folder %s mukht kiya gaya %s mukht kiye gaya 'q' -chodna (%d directoriyan hatayi gayi) (%d directory hatayi gayi) (%d failen hatayi gayi) (%d file hatayi gayi) (malik ki parikshya/permissions likhen)
 -i / --interactive: abhi tak kiya nahi gaya
 -r / --recurse: nahi kiya gaya
 <CURSOR-RIGHT / CURSOR-LEFT> - svich abhisht folders <SPACE> - cheezein chunit or chayan radd karen karte wakt / <ENTER> -chunit cheezein bahal karen == shamil %d file == == failen shamil %d hai == karya band kiya gaya. Jari rakhen? (haan/na):  Banaya gaya %s
 Yeh directory banayi gayi %s
 configuration file banate waqt: Directory sudhikaran na ho -failen yahan hai 
 behrupi file ka naam hai manzil par - sanlagn wakht par string 
 File nahi mili: '%s'
 Sahi aadesh nahi diya gaya;
Dabaye '%s -h' adhik jankari ke liye
 
 Galat apshist option: '%s'
 jyada se jyada %u hua, chod dijiye
 koi sahi filesystem nahi mile "%s"
 sari failen ki sudhikaran kiya jayega apshist folders …
 sudhikaran files din ginke ke baad apshist folder se hogi (%u) …
 Padh raha hai %s…
 Galti yahan kare <%s>.
 Restore() aaya %d
 Khoja jaa raha hai basefile naam istemal karke…
 Easter Bunny bola, "Hello, Duniya." Apshist folder may jo hai abhi hataya jayega Jaari nahi rakh saka. Bahar jaa raha hai….
 Nahi padha or likha jaa raha configuration file
 Unjana ya galat bikalp: '%s'
 Istemal: rmw[Bikalp]… FILE...
hataye file ya failon ko apshist directory, configuration file may

   ya: rmw -s
   ya: rmw -u
   ya: rmw -z FILE...
bahal karen filon ko apshist directory se

  -h, --madad
  -c, --config filename    madad aayegaa configuration ke liye
  -l, --list               list apshist directories
  -g, --sudhokaran             sudhikaran karen tab bhi agar aaj hua hai
  -o, --akela          dekhen akele files ko(maintenance)
  -f, --bal            rmw ko sudhikaran karen piche
  -e. --khali           sab khali hai (sudhikaran) sare apshist folder ko
  -v, --vaachal          nikalne wale messages badhaye
  -w, --warranty         dikhaye warranty
  -V, --sanskaran        dikhaye sanskaran aur licence ki jankari
 jodagaya config file: %s
 dry-run mode hai hataya nahi jaa saka %s
 praroop .trashinfo file ki 'file %s' galat hai samrooh <%s> koi kaam ka apshisht folder nahi mila
Aapko binti hai ki configuration file and anumati dekhen
agar aapko madad ki ya koi galti batani hai
aap rmw web site par karen
 sudhikaran nahi kiya gaya: -f or --force ka istemal karen
 sudhikaran nahi kiya jaa sakta('purge_after' ko '0' karen)
 realpath() may error mila hai
 hataya ja sakta hai,  rmw: %s(): buffer bhut chota hai (mila %d, chahiye tha %d)
 dhundhna purna hua
 config aur directory nahi bante wakht
configuration file and permissions ko dekhe

 memory avantit prayas karte wakht -- %s:L%d
 badalte wakht anumati %s
 band karte waqt %s
 banate wakht %s
 home directory ka rasta milte wakht
 kholte wakt %s
 kholte wakht %s -- %sL%d
 hatate wakht %s
 .trashinfo fail hatate wakht:'%s'
 hatane se pehle (rename) karen
 %s -> %s -- %s:L%d

 