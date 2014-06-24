<style type="text/css">
{literal}
  table > tbody > tr.error > td {
    background:#F99 none repeat scroll 0 0;
  }
  table > tbody > tr.success > td {
    background:#9F9 none repeat scroll 0 0;
  }
{/literal}
</style><div></div>
<fieldset id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <legend><span>{$section}</span></legend>
  <div>
    <p class="warning">{t}Warning : Once you import your OPSI hosts into FusionDirectory, they will be managed by it, so if you delete them or deactivate their OPSI tab, they will be removed from OPSI.{/t}</p>
    <table>
      {foreach from=$attributes item=attribute key=id}
        <tr>
          <td title="{$attribute.description}"><label for="{$attribute.htmlid}">{eval var=$attribute.label}</label></td>
          <td>{eval var=$attribute.input}</td>
        </tr>
      {/foreach}
      {if $importResult}
        <tr>
          <td colspan="2">
            <table class="listingTable">
              <thead>
                <tr>
                  <th><img src="images/info_small.png"/></th>
                  <th>Host</th>
                  <th>Details</th>
                </tr>
              </thead>
              <tbody>
                {foreach from=$importResult item=message key=id}
                  {if $message === TRUE}
                    <tr class="success">
                      <td><img src="geticon.php?context=status&icon=task-complete&size=16"/></td>
                      <td>{$id}</td>
                      <td>saved</td>
                    </tr>
                  {else}
                    <tr class="error">
                      <td><img src="geticon.php?context=status&icon=task-failure&size=16"/></td>
                      <td>{$id}</td>
                      <td>
                        {foreach from=$message item=line}
                          {$line}<br/>
                        {/foreach}
                      </td>
                    </tr>
                  {/if}
                {/foreach}
              </tbody>
            </table>
          </td>
        </tr>
      {/if}
    </table>
  </div>
</fieldset>
