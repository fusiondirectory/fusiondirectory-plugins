<br>
<h2>{t}Argonaut server{/t}</h2>
<br>
{render acl=$argonautACL}
  <input id="argonautDeleteFinished" name = "argonautDeleteFinished" type="checkbox"
   {if $argonautDeleteFinished==1} checked="checked"{/if} />
   <label for="argonautDeleteFinished">Delete finished tasks</label><br>
  <label for="argonautPort">Port :</label>
  <input id="argonautPort" name = "argonautPort" type="number" value="{$argonautPort}" />
  
{/render}

<p class="seperator">&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
  <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
  &nbsp; 
  <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'> 
</div>
<input type="hidden" name="argonautServerPosted" value="1">
