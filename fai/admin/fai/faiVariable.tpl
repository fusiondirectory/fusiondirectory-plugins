<table summary="" width="100%">
	<tr>
		<td width="50%" valign="top">
				<h2><img class="center" alt="" src="plugins/fai/images/fai_small.png" align="middle" title="{t}Generic{/t}">&nbsp;{t}Generic{/t}</h2>
				<table summary="" cellspacing="4">
					<tr>
						<td>
							<LABEL for="cn">
							{t}Name{/t}{$must}
							</LABEL>
						</td>
						<td>
{render acl=$cnACL}
							<input type='text' value="{$cn}" size="45" maxlength="80" disabled id="cn">
{/render}
						</td>
					</tr>
					<tr>
						<td>
							<LABEL for="description">
							{t}Description{/t}
							</LABEL>
						</td>
						<td>
{render acl=$descriptionACL}
							<input type='text' size="45" maxlength="80" value="{$description}" name="description" id="description">
{/render}
						</td>
					</tr>
				</table>
		</td>
		<td style="border-left: 1px solid rgb(160, 160, 160);">
		   &nbsp;
	 	</td>
		<td>
				<h2><img class="center" alt="" src="plugins/fai/images/fai_variable.png" align="middle" title="{t}Objects{/t}">&nbsp;
					<LABEL for="SubObject">
						{t}List of assigned variables{/t}
					</LABEL>
				</h2>
				<table summary="" width="100%">
				<tr>
					<td>
						<select name="SubObject[]" multiple  title="{t}Choose a variable to delete or edit{/t}" 
							style="width:100%;" size="20" id="SubObject">
							{html_options options=$SubObjects}
						</select><br>
{if $is_createable}
					<input type="submit" name="AddSubObject"     value="{msgPool type=addButton}"		title="{msgPool type=addButton}">
{else}
					<input disabled type="submit" name="AddSubObject"     value="{msgPool type=addButton}"		title="{msgPool type=addButton}">
{/if}
					<input type="submit" name="EditSubObject"    value="{t}Edit{/t}"    title="{t}Edit{/t}" >
{if $is_removeable}
					<input type="submit" name="DelSubObject"     value="{msgPool type=delButton}"  title="{msgPool type=delButton}">
{else}
					<input disabled type="submit" name="DelSubObject"     value="{msgPool type=delButton}"  title="{msgPool type=delButton}">
{/if}
					</td>
				</tr>
				</table>
		</td>
	</tr>
</table>
<input type="hidden" value="1" name="FAIvariable_posted">
<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('cn','description');
  -->
</script>

