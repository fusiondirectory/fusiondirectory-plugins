<input type="hidden" name="SubObjectFormSubmitted" value="1">

<h2><img class="middle" alt="" src="images/forward.png" title="{t}Generic{/t}">&nbsp;{t}Generic{/t}</h2>
<table width="100%" summary="">
<tr>
  <td style="vertical-align:top;width:50%;border-right:1px solid #B0B0B0; padding-right:10px;">
		<table style='width:100%;'>
		  <tr>
		    <td>
		{t}File name{/t}{$must}&nbsp;
		    </td>
		    <td style='width:100%;'>
{render acl=$cnACL}
		      <input style='width:100%;' value="{$templateFile}" 
            name="templateFile" maxlength="80">
{/render}
		    </td>
	    </tr>
		  <tr>
			  <td>
				  <LABEL for="templatePath">
				  	{t}Destination path{/t}{$must}&nbsp;
				  </LABEL>
			  </td>
			  <td>
{render acl=$FAItemplatePathACL}
				  <input style='width:100%;' type="text" name="templatePath" 
            value="{$templatePath}" id="templatePath" >
{/render}
				</td>
      </tr>
		</table>
	</td>
  <td style='vertical-align:top;'>
		<table style='width:100%;'>
      <tr>
       	<td style='vertical-align:top;'>
          {t}Description{/t}
        </td>
        <td style='width:100%;'>
{render acl=$descriptionACL}
		      <input  style='width:100%;' maxlength="80" 
            value="{$description}" name="description">
{/render}
	      </td>
      </tr>
    </table>
  </td>
</tr>
</table>

<p class="seperator">&nbsp;</p>

<table width="100%" summary="">
<tr>
  <td colspan=2>
    <h2>
      <img class="center" alt="" 
        src="plugins/fai/images/fai_template.png" 
        title="{t}Template attributes{/t}">&nbsp;{t}Template attributes{/t}
    </h2>
  </td>
</tr>
<tr>
  <td style="vertical-align:top;width:50%;border-right:1px solid #B0B0B0">
  <table summary="">
    <tr>
      <td>
        {t}File{/t}{$must}:&nbsp; {$status}
        {if $bStatus}
          <a href="{$plug}&amp;getFAItemplate">
          <img class="center" alt="{t}Save template{/t}..." 
            title="{t}Save template{/t}..." src="images/save.png" border="0" />
          </a>
          <a href="{$plug}&amp;editFAItemplate">
          <img class="center" alt="{t}Edit template{/t}..." 
            title="{t}Edit template{/t}..." src="images/lists/edit.png" border="0" />
          </a>
        {/if}
      </td>
    </tr>
    {if $bStatus}
    <tr>
      <td>
		    {t}Full path{/t}:&nbsp; <i>{$FAItemplatePath}</i>
      </td>
    </tr>
    {/if}
    <tr>
		  <td style="vertical-align:top" class="center">
{render acl=$FAItemplateFileACL}
			  <input type="file" name="FAItemplateFile" value="" id="FAItemplateFile">
{/render}
{render acl=$FAItemplateFileACL}
			  <input type="submit" value="{t}Upload{/t}" name="TmpFileUpload">
{/render}
		  </td>
    </tr>
	</table>
	</td>
	<td>
	  <table summary="">
		<tr>
	
		<td>
			<LABEL for="user">
				{t}Owner{/t}{$must}&nbsp;
			</LABEL>
			</td>
		<td>
{render acl=$FAIownerACL}
			<input type="text" name="user" value="{$user}" id="user" size="15">
{/render}
		</td>
	</tr><tr>
		<td style="vertical-align:top">
			<LABEL for="group">
				{t}Group{/t}{$must}&nbsp;
			</LABEL>
			</td>
		<td>
{render acl=$FAIownerACL}
			<input type="text" name="group" value="{$group}" id="group" size="15">
{/render}
			<br>
			<br>
		</td>
	</tr><tr>
		<td style="vertical-align:top">{t}Access{/t}{$must}&nbsp; </td>
		<td>
	  <table summary="" style="border:1px solid #B0B0B0">
	       <colgroup width="55" span="3">
	        </colgroup>
		<tr>
			<th>{t}Class{/t}</th>
			<th>{t}Read{/t}</th>
			<th>{t}Write{/t}</th>
			<th>{t}Execute{/t}</th>
			<th>&nbsp;</th>
			<th>{t}Special{/t}</th>
			<th>&nbsp;</th>
		</tr>
		<tr><td>{t}User{/t}</td>
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="u4" value="4" {$u4}></td>
{/render}
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="u2" value="2" {$u2}></td>
{/render}
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="u1" value="1" {$u1}></td>
{/render}
			<td>&nbsp;</td>
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="s4" value="4" {$s4}></td>
{/render}
			<td>({t}SUID{/t})</td>
			</tr>

		<tr><td>{t}Group{/t}</td>
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="g4" value="4" {$g4}></td>
{/render}
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="g2" value="2" {$g2}></td>
{/render}
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="g1" value="1" {$g1}></td>
{/render}
			<td>&nbsp;</td>
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="s2" value="2" {$s2}></td>
{/render}
			<td>({t}SGID{/t})</td>
			</tr>

		<tr><td>{t}Others{/t}</td>
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="o4" value="4" {$o4}></td>
{/render}
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="o2" value="2" {$o2}></td>
{/render}
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="o1" value="1" {$o1}></td>
{/render}
			<td>&nbsp;</td>
{render acl=$FAImodeACL}
			<td align="center"><input type="checkbox" name="s1" value="1" {$s1}></td>
{/render}
			<td>({t}sticky{/t})</td>
	</tr></table>
	
		</td></tr></table>
	</td>
</tr>
<tr>
<td colspan=2>
<br>
<p class="seperator">&nbsp;</p>
<br>
<div style="align:right;" align="right">
{if !$freeze}
	<input type="submit" value="{msgPool type=applyButton}" 	name="SaveSubObject">&nbsp;
{/if}
	<input type="submit" value="{msgPool type=cancelButton}" 	name="CancelSubObject">
</div>
</td>
</tr>
</table>

<input type='hidden' name='FAItemplateEntryPosted' value='1'>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('cn','description');
  -->
</script>

