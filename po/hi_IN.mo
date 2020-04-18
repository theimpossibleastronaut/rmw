��    ,      |  ;   �      �  �   �  �  W  K    �   j  �   �  Y   �  
   �     
	     $	  @   0	  �   q	  2   �	     2
  Q   Q
     �
  
   �
  /   �
  $   �
        $   6      [  3   |  H   �  .   �     (     :     L     X  $   n  ,   �  =   �     �  a     .   q  !   �     �     �  .   �          (     D  %   W  3   }  H  �  �   �  4  �    �  �   V  �   �  N   �     �     �       *     �   F  7   �  !     *   1     \     p     |     �  (   �  ,   �       4   '  `   \     �     �     �     �          )  .   H  ;   w     �  S   �  "        >     X     i  $   w     �     �     �     �  4   �              !      *      +   ,             %                                  #                )                
                        &          $   	                                   '   (      "       
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
 %d directories skipped (contains read-only files)
 %d file purged %d files purged %d file was removed to the waste folder %d files were removed to the waste folder %s freed %s freed 'q' - quit (%d directory deleted) (%d directories deleted) (%d file deleted) (%d files deleted) (check owner/write permissions)
 -i / --interactive: not implemented
 -r / --recurse: not implemented
 <CURSOR-RIGHT / CURSOR-LEFT> - switch waste folders <SPACE> - select or unselect an item. / <ENTER> - restore selected items == contains %d file == == contains %d files == Action cancelled. Continue? (y/n):  Created %s
 Created directory %s
 Creating default configuration file: Directory not purged - still contains files
 rmw: %s(): buffer too small (got %d, needed a minimum of %d)
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
PO-Revision-Date: 2020-04-17 23:53+0530
Language-Team: 
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=2; plural=(n==0 || n==1);
X-Generator: Poedit 2.3
Last-Translator: 
Language: hi_IN
 # Kitne dino tak failonko ko pehle apshist folder may rehne ki anumati di jani chahiye?
# Yeh sthayi roop se hata di jati hai
#
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
 
 # shudh 'f' bikalp ke bina chalane ki anumati hai yadi | agar appko
# 'f' ka prayog karna hai to niche Rekha par comment karen 
#
 

 	 ===] Bahal [===

-z, --bahal <wildcard filename(s) pattern>
-s ,--bahal karneke liye suchi se chuninda         failon ka chayan karen
-u, --purbith-pichle          purbit pichle ReMove
 

adhik madad aur jankari ke liye rmw homepage jaaye
aur samtharna prapt karen    :galati:   :return code: %d -- %s
  :chetawani:   <---> galati ke karan bana string <--->

 # Dhyan de :Agar do aphsist folder ek file system may hai, rmw unko hata dega
# pehle aphsist folder ko likhke, dusre ko nazarandaz kiya jayega
 %d directories chodi gayi hai (sirf padhne wali files)
 %d failen sudhikaran ki gayi hai  %d failen hatayi gayi hai waste folder ko  %s mukht kiya gaya  'q' -chodna (%d directory hatayi gayi)  (%d failen hatayi gayi)  (malik ki parikshya/permissions likhen)
 -i / --interactive: abhi tak kiya nahi gaya
 -r / --recurse: nahi kiya gaya
 <CURSOR-RIGHT / CURSOR-LEFT> - svich abhisht folder  <SPACE> - cheezein chunit or chayan radd karen karte wakt / <ENTER> -chunit cheezein bahal karen == shamil %d file ==  karya band kiya gaya Jari rakhen? (haan/na) banaya gaya %s
 directory banayi gayi %s
 configuration file banate waqt Directory sudhikaran na ho -failen yahan hai 
 rmw: %s(): buffer bhut chota hai (mila %d, chahiye tha %d)
 dhundhna purna hua
 config aur directory nahi bante wakht
configuration file and permissions ko dekhe

 memory avantit prayas karte wakht
 badalte wakht anumati %s
 band karte waqt
 banate wakht
 home directory ka rasta milte wakht
 kholte wakt %s
 kholte wakht %s -- %sL%d
 hatate wakht %s
 .trashinfo fail hatate wakht
 hatane se pehle (rename) karen
 %s -> %s -- %s:L%d

 