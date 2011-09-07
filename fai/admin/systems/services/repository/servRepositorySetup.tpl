<h2><img src="plugins/fai/images/fai_small.png" alt=''>&nbsp;{t}Repository{/t}</h2>

<table width="100%" summary=''>
	<tr>
		<td width="50%" valign="top" style="border-right:1px solid #A0A0A0">
			<table summary=''>
				<tr>
					<td>{t}Parent server{/t}
					</td>
					<td>
{render acl=$ParentServerACL}
						<select name="ParentServer">
							{html_options options=$ParentServers values=$ParentServerKeys selected=$ParentServer} 
						</select>
{/render}
					</td>
				</tr>
				<tr>
					<td><label for="Release">{t}Release{/t}</label>
					</td>
					<td>
{render acl=$ReleaseACL}
						<input type="text" value="{$Release}" name="Release" id="Release">
{/render}
					</td>
				</tr>
				<tr>
					<td><label for="Url">{t}URL{/t}</label>
					</td>
					<td>
{render acl=$UrlACL}
						<input type="text" size="40" value="{$Url}" name="Url" id="Url">
{/render}
					</td>
				</tr>
				<tr>
					<td><label for="Install">{t}Installation mirror{/t}</label>
					</td>
					<td>
{render acl=$UrlACL}
						<input type="checkbox" {if $Install=="install"}checked{/if} name="Install" id="Install">
{/render}
					</td>
				</tr>
				<tr>
					<td><label for="Local">{t}Local mirror{/t}</label>
					</td>
					<td>
{render acl=$UrlACL}
						<input type="checkbox" {if $Local=="local"}checked{/if} name="Local" id="Local">
{/render}
					</td>
				</tr>
			</table>
		</td>
		<td>
			{t}Sections{/t}<br>
{render acl=$SectionACL}
			{$Sections}
{/render}
{render acl=$SectionACL}
			<input type="text" 	name="SectionName" value="" style='width:100%;'>
{/render}
{render acl=$SectionACL}
			<input type="submit" 	name="AddSection"  value="{msgPool type=addButton}">
{/render}
		</td>
	</tr>
</table>
<input type='hidden' name='servRepositorySetup_Posted' value='1'>
<p class="plugbottom">
  <input type=submit name="repository_setup_save" value="{msgPool type=applyButton}">
  &nbsp;
  <input type=submit name="repository_setup_cancel" value="{msgPool type=cancelButton}">
</p>

