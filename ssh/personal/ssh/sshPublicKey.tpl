<p class="contentboxh" style="font-size:12px">
  <b>{t}List of SSH public keys for this user{/t}</b><br>
</p>
<p class="contentboxb" style="border-top:1px solid #B0B0B0;background-color:#F8F8F8">
  <select style="width:100%; margin-top:4px; height:450px;" name="keylist[]" size="15" multiple>
     {html_options options=$keylist}
  </select>
</p>
{render acl=$sshPublicKeyACL}
<input type=file name="key">
&nbsp;
<input type=submit name="upload_sshpublickey" value="{t}Upload key{/t}">
&nbsp;
<input type=submit name="remove_sshpublickey" value="{t}Remove key{/t}">
{/render}

<p class="plugbottom">
  <input type=submit name="save_sshpublickey" value="{msgPool type=saveButton}">
  &nbsp;
  <input type=submit name="cancel_sshpublickey" value="{msgPool type=cancelButton}">
</p>
