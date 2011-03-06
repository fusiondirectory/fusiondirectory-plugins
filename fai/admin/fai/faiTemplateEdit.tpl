<h2>Template entry</h2>

{if !$mb_extension}

{msgPool type=missingext param1='multi byte'}

<p class='seperator'>
  <div style='text-align:right;'>
    <input type='submit' name='templateEditCancel' value='{msgPool type=cancelButton'}'>
  </div>
</p>

{else}

{if $write_protect}
{t}This FAI template is write protected. Editing may break it!{/t}
<br><input type='submit' value='{t}Edit anyway{/t}' name='editAnyway'>
{/if}

<textarea {if $write_protect} disabled {/if}
  style='width:100%; height: 350px;'
  {if !$write_protect}name="templateValue"{/if}
>{$templateValue}</textarea>

<p class='seperator'>
  <div style='text-align:right;'>
    <input type='submit' name='templateEditSave' value='{msgPool type=okButton}'>
    &nbsp;
    <input type='submit' name='templateEditCancel' value='{msgPool type=cancelButton'}'>
  </div>
</p>

{/if}
