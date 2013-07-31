# This is taken from Agent/Inventory.pm
my %fields = (
    ANTIVIRUS   => [ qw/COMPANY NAME GUID ENABLED UPTODATE VERSION/ ],
    BATTERIES   => [ qw/CAPACITY CHEMISTRY DATE NAME SERIAL MANUFACTURER
                        VOLTAGE/ ],
    CONTROLLERS => [ qw/CAPTION DRIVER NAME MANUFACTURER PCICLASS VENDORID
                        PRODUCTID PCISUBSYSTEMID PCISLOT TYPE REV/ ],
    CPUS        => [ qw/CACHE CORE DESCRIPTION MANUFACTURER NAME THREAD SERIAL
                        STEPPING FAMILYNAME FAMILYNUMBER MODEL SPEED ID EXTERNAL_CLOCK ARCH/ ],
    DRIVES      => [ qw/CREATEDATE DESCRIPTION FREE FILESYSTEM LABEL LETTER
                        SERIAL SYSTEMDRIVE TOTAL TYPE VOLUMN/ ],
    ENVS        => [ qw/KEY VAL/ ],
    INPUTS      => [ qw/NAME MANUFACTURER CAPTION DESCRIPTION INTERFACE LAYOUT
                        POINTINGTYPE TYPE/ ],
    MEMORIES    => [qw/CAPACITY CAPTION FORMFACTOR REMOVABLE PURPOSE SPEED
                       SERIALNUMBER TYPE DESCRIPTION NUMSLOTS MEMORYCORRECTION
                       MANUFACTURER/ ],
    MODEMS      => [ qw/DESCRIPTION NAME/ ],
    MONITORS    => [ qw/BASE64 CAPTION DESCRIPTION MANUFACTURER SERIAL
                        UUENCODE/ ],
    NETWORKS    => [ qw/BSSID DESCRIPTION DRIVER FIRMWARE IPADDRESS IPADDRESS6
                        IPDHCP IPGATEWAY IPMASK IPMASK6 IPSUBNET IPSUBNET6
                        MANAGEMENT MANUFACTURER MACADDR MODEL MTU PCISLOT
                        PNPDEVICEID STATUS SLAVES SPEED SSID TYPE VIRTUALDEV
                        WWN/ ],
    PORTS       => [ qw/CAPTION DESCRIPTION NAME TYPE/ ],
    PROCESSES   => [ qw/USER PID CPUUSAGE MEM VIRTUALMEMORY TTY STARTED CMD/ ],
    REGISTRY    => [ qw/NAME REGVALUE HIVE/ ],
    RUDDER      => [ qw/AGENT UUID HOSTNAME/ ],
    SLOTS       => [ qw/DESCRIPTION DESIGNATION NAME STATUS/ ],
    SOFTWARES   => [ qw/COMMENTS FILESIZE FOLDER FROM HELPLINK INSTALLDATE NAME
                        NO_REMOVE RELEASE_TYPE PUBLISHER UNINSTALL_STRING
                        URL_INFO_ABOUT VERSION VERSION_MINOR VERSION_MAJOR
                        GUID ARCH USERNAME USERID/ ],
    SOUNDS      => [ qw/CAPTION DESCRIPTION MANUFACTURER NAME/ ],
    STORAGES    => [ qw/DESCRIPTION DISKSIZE INTERFACE MANUFACTURER MODEL NAME
                        TYPE SERIAL SERIALNUMBER FIRMWARE SCSI_COID SCSI_CHID
                        SCSI_UNID SCSI_LUN WWN/ ],
    VIDEOS      => [ qw/CHIPSET MEMORY NAME RESOLUTION PCISLOT/ ],
    USBDEVICES  => [ qw/VENDORID PRODUCTID MANUFACTURER CAPTION SERIAL CLASS
                        SUBCLASS NAME/ ],
    USERS       => [ qw/LOGIN DOMAIN/ ],
    LOCAL_USERS  => [ qw/LOGIN ID NAME HOME SHELL/ ],
    LOCAL_GROUPS => [ qw/NAME ID MEMBER/ ],
    PRINTERS    => [ qw/COMMENT DESCRIPTION DRIVER NAME NETWORK PORT RESOLUTION
                        SHARED STATUS ERRSTATUS SERVERNAME SHARENAME
                        PRINTPROCESSOR SERIAL/ ],
    BIOS             => [ qw/SMODEL SMANUFACTURER SSN BDATE BVERSION
                             BMANUFACTURER MMANUFACTURER MSN MMODEL ASSETTAG
                             ENCLOSURESERIAL BIOSSERIAL
                             TYPE SKUNUMBER/ ],
    HARDWARE         => [ qw/USERID OSVERSION PROCESSORN OSCOMMENTS CHECKSUM
                             PROCESSORT NAME PROCESSORS SWAP ETIME TYPE OSNAME
                             IPADDR WORKGROUP DESCRIPTION MEMORY UUID VMID DNS
                             LASTLOGGEDUSER USERDOMAIN DATELASTLOGGEDUSER
                             DEFAULTGATEWAY VMSYSTEM WINOWNER WINPRODID
                             WINPRODKEY WINCOMPANY WINLANG CHASSIS_TYPE VMID
                             VMNAME VMHOSTSERIAL/ ],
    OPERATINGSYSTEM  => [ qw/KERNEL_NAME KERNEL_VERSION NAME VERSION FULL_NAME
                            SERVICE_PACK INSTALL_DATE FQDN DNS_DOMAIN
                            SSH_KEY ARCH BOOT_TIME/ ],
    ACCESSLOG        => [ qw/USERID LOGDATE/ ],
    VIRTUALMACHINES  => [ qw/MEMORY NAME UUID STATUS SUBSYSTEM VMTYPE VCPU
                             VMID MAC COMMENT OWNER SERIAL/ ],
    LOGICAL_VOLUMES  => [ qw/LV_NAME VGN_AME ATTR SIZE LV_UUID SEG_COUNT
                             VG_UUID/ ],
    PHYSICAL_VOLUMES => [ qw/DEVICE PV_PE_COUNT PV_UUID FORMAT ATTR
                             SIZE FREE PE_SIZE VG_UUID/ ],
    VOLUME_GROUPS    => [ qw/VG_NAME PV_COUNT LV_COUNT ATTR SIZE FREE VG_UUID
                             VG_EXTENT_SIZE/ ],
    LICENSEINFOS     => [ qw/NAME FULLNAME KEY COMPONENTS TRIAL UPDATE OEM ACTIVATION_DATE PRODUCTID/ ]
);

my %fields2;

while (my ($a, $b) = each(%fields)) {
  foreach my $c (@$b) {
    $c =~ s/_//g;
  }
  @fields2{@$b} = ();
}

print "##\n## inventory-fd.schema - Needed by Fusion Directory for managing inventories\n##\n";
print "\n# Attributes\n";

my $i = 1;

for my $f (keys(%fields2)) {
  print "attributetype ( 1.3.6.1.4.1.38414.39.1.$i NAME 'fdInventory$f'
  DESC 'FusionDirectory - inventory, $f'
  EQUALITY caseExactMatch
  SUBSTR caseExactSubstringsMatch
  SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 )\n\n";
  $i++;
}

print "\n# Object classes\n";
print "objectclass ( 1.3.6.1.4.1.38414.39.2.1 NAME 'fdInventoryContent'
  DESC 'FusionDirectory inventory information'
  MUST ( cn )
  MAY ( macAddress ) )\n\n";

$i = 2;

while (my ($a, $b) = each(%fields)) {
  $a =~ s/_//g;
  print "objectclass ( 1.3.6.1.4.1.38414.39.2.$i NAME 'fdInventory$a'
  DESC 'FusionDirectory inventory information - $a'
  MUST ( cn )
  MAY ( ";
  foreach my $c (@$b) {
    $c =~ s/_//g;
    $c = "fdInventory$c";
  }
  print join(' $ ',@$b);
  print " ) )\n\n";
  $i++;
}
