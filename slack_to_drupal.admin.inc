<?php

function slack_to_drupal_admin_config() {

  $form = array();

  $form['slack_to_drupal_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Slack Token'),
    '#default_value' => variable_get('slack_to_drupal_token'),
  );

  $form['slack_to_drupal_channel_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Slack Channel'),
    '#default_value' => variable_get('slack_to_drupal_channel_id'),
  );

  $form['slack_to_drupal_folder_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Local path to files'),
    '#default_value' => variable_get('slack_to_drupal_folder_name'),
  );

  return system_settings_form($form);
}

function slack_to_drupal_approve(){
  $query = new EntityFieldQuery();
  $ids = array();

  $query->entityCondition('entity_type', 'file')
   	->propertyCondition('type', 'slack_image')
   	->fieldCondition('slack_image_approved', 'value', '0', '=');
  $result = $query->execute();

  if (isset($result['file'])) {
   	$ids = array_keys($result['file']);
    // Adds CSS
    drupal_add_css($data['css'], array('type' => 'inline'));
    drupal_add_css(drupal_get_path('module', 'slack_to_drupal') . '/css/slack_to_drupal.css');
  }

  $fids = array();
  $count = 0;
  foreach($ids as $id){
  	$file = file_load($id);
  	$fileview = file_view($file, 'slack_image_display');
  	$results[] = drupal_render($fileview);
  	$fids[$count] = $id;
  	$count++;
  }

  $form['fids'] = array(
	'#title' =>  'fids',
	'#value' => $fids,
	'#type' => 'value',
  );

  $form['files'] = array(
  	'#title' => t('Unapproved Images'),
  	'#type' => 'checkboxes',
  	'#options' => $results,
  );

  $form['approve'] = array(
    '#type' => 'submit',
    '#value' => t('Approve'),
  );

  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  return $form;
}

function slack_to_drupal_approve_validate($form, &$form_state) {
}

function slack_to_drupal_approve_submit($form, &$form_state) {
  foreach($form_state['input']['files'] as $fid){
  	if(isset($fid)){
  		$image = file_load($form_state['values']['fids'][$fid]);
  		if($form_state['values']['op'] == 'Approve'){
  			$image->slack_image_approved['und']['0']['value'] = 1;
  			file_save($image);
  			 drupal_set_message('Selected files are approved');
  		}else{
  			file_delete($image);
  			drupal_set_message('Selected files are removed');
  		}
  	}
  }
}
