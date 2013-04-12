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
    <tr>
      <td colspan="2">
        {$opsiImport_result}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
