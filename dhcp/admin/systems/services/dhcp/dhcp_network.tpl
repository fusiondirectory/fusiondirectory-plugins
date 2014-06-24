{* FusionDirectory dhcp sharedNetwork - smarty template *}
<table width="100%">
 <tr>
  <td width="50%" style="vertical-align:top">
   <p><b>{t}Network configuration{/t}</b></p>
   <table>
    <tr>
     <td>{t}Router{/t}</td>
     <td>
{render acl=$acl}
      <input id='routers' type='text' name='routers' size='25' maxlength='80' value='{$routers}'
              title='{t}Enter name or IP address of router to be used in this section{/t}'>
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Netmask{/t}</td>
     <td>
{render acl=$acl}
      <input type='text' name='subnet_mask' size='18' maxlength='15' value='{$subnet_mask}'>
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Broadcast{/t}</td>
     <td>
{render acl=$acl}
      <input type='text' name='broadcast_address' size='18' maxlength='15' value='{$broadcast_address}'>
{/render}
     </td>
    </tr>
   </table>
   <br>
   <br>
   <p><b>{t}Bootup{/t}</b></p>
   <table>
    <tr>
     <td>{t}Filename{/t}</td>
     <td>
{render acl=$acl}
      <input type='text' name='filename' size='25' maxlength='80' value='{$filename}'
              title='{t}Enter name of file that will be loaded via tftp after client has started{/t}'>
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Next server{/t}</td>
     <td>
{render acl=$acl}
      <input type='text' name='nextserver' size='25' maxlength='80' value='{$nextserver}'
              title='{t}Enter name of server to retrieve bootimages from{/t}'>
{/render}
     </td>
    </tr>
   </table>

  </td>
  <td style="vertical-align:top">
   <p><b>{t}Domain Name Service{/t}</b></p>
   <table>
    <tr>
     <td>{t}Domain{/t}</td>
     <td>
{render acl=$acl}
      <input type='text' name='domain' size='25' maxlength='80' value='{$domain}'
              title='{t}Name of domain{/t}'>
{/render}
     </td>
    </tr>
    <tr>
     <td colspan=2>
      <br>
      {t}DNS server{/t}<br>
{render acl=$acl}
      <select name='dnsserver'  title='{t}List of DNS servers to be propagated{/t}' style="width:350px;" size="4">
       {html_options options=$dnsservers}
      </select>
{/render}
      <br>
{render acl=$acl}
      <input type='text' name='addserver' size='25' maxlength='80' title='{t}DNS server do be added{/t}'>&nbsp;
{/render}
{render acl=$acl}
      <input type='submit' name='add_dns' value='{msgPool type=addButton}' title='{t}Click here add the selected server to the list{/t}'>
{/render}
{render acl=$acl}
      <input type='submit' name='delete_dns' value='{msgPool type=delButton}' title='{t}Click here remove the selected servers from the list{/t}'>
{/render}
     </td>
    </tr>
    <tr>
     <td colspan=2>
      <p><b>{t}Domain Name Service options{/t}</b></p>
{render acl=$acl}
      <input type=checkbox name="autohost" value="1" {$autohost}>{t}Assign hostnames found via reverse mapping{/t}
{/render}
      <br>
{render acl=$acl}
      <input type=checkbox name="autohostdecl" value="1" {$autohostdecl}>{t}Assign hostnames from host declarations{/t}
{/render}
     </td>
    </tr>
   </table>

  </td>
 </tr>
</table>
<!-- Place cursor in correct field -->
<script type="text/javascript">
  <!-- // First input field on page
     focus_field('cn','routers');
  -->
</script>

