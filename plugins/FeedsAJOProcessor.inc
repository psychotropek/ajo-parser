<?php

/**
 * @file
 * Contains FeedsAJOProcessor.
 */

/**
 * Allows a feed node to populate fields on itself.
 */
class FeedsAJOProcessor extends FeedsNodeProcessor {

  /**
   * Overrides parent::process().
   */
  public function process(FeedsSource $source, FeedsParserResult $parser_result) {
    
    //todo: while parser result has more?
    $item = $parser_result->shiftItem();

    $values = array(
      'type' => 'article',
      'uid' => 1,
      'status' => 1,
      'comment' => 1,
      'promote' => 0,
    );
    $entity = entity_create('node', $values);
    $ewrapper = entity_metadata_wrapper('node', $entity);
    $ewrapper->title->set($item['title']);
    $ewrapper->body->set(array('value' => $item['body']));
    //the abstract is contained in the summary field
    $ewrapper->field_article_summary->set(array('value' => $item['summary']) );
    $ewrapper->field_article_teaser->set(array('value' => $item['teaser']));
    $ewrapper->field_article_doi->set($item['digital_object_id']);
   
    //n.b.Legacy authors is a rich text that contains author disclosure
    $ewrapper->field_article_legacy_authors->set($item['author_disclosure']);

    $file_efq = new EntityFieldQuery();
    //todo: will need two filenames, will need to execute this for both medium and big image
    $filename = "figure1.jpg";
    $file_efq->entityCondition('entity_type', 'file')
      ->propertyCondition('filename', $filename);
    $file_efq_result = $file_efq->execute();
    $fid = 0;
    if (count($file_efq_result)) {
      $fid = array_pop($file_efq_result['file'])->fid;
    }
    // Set both file and title separately. TODO: need actual file
    $image_obj = file_load($fid);
    $ewrapper->field_article_medium_image->file = $image_obj;
    $ewrapper->field_article_medium_image->title = $title;
  
    //$ewrapper->field_article_medium_image->set();
    //$ewrapper->field_article_big_image->set();
    
    
    //Inside the article- all long text fields. Key Info = Take Home Points
    $ewrapper->field_article_key_info->set(array('value' => $item['take_home_points']));
    
    //TODO: Figures and tables requires several fields. We're going to have to add something in the $items
    //add figures first, then tables. We'll see how they 
    $figures_tables = "";
    foreach ($item['figures'] as $figure)
    {
      //todo: add html, filepath, caption
      //todo: the gallery link/code is inserted via colorbox module. But what invokes the module? Check theme if doesn't auto-format.
      //element has div media media-element-container media-medstat_image_centered
      //<a href="https://dev-ajo-test.pantheonsite.io/sites/default/files/norris0318_f1.jpg" 
      //title="" class="colorbox init-colorbox-processed cboxElement" data-colorbox-gallery="gallery-file-210130-dXOw065YekU">
      //<img class="media-element file-medstat-image-centered" data-delta="1" typeof="foaf:Image" 
      //src="https://dev-ajo-test.pantheonsite.io/sites/default/files/styles/large/public/norris0318_f1.jpg?itok=25mKu7Br" 
      //width="480" height="135" alt="" title=""></a>
      //figure 1 should be text right before caption, below image

    }
    foreach ($item['tables'] as $table)
    {
      //todo: add html- will just be the table code?
            //above_table should start with Table X. See if they put it in their document?
      //output $table[above_table], table_data, and table_caption
    }

    $ewrapper->field_article_figures_tables->set(array('value' => $figures_tables));
    
    
    
    
    //TODO: What is the multimedia going to be? How do they want to insert it?
    //$ewrapper->field_article_multimedia->set("some code here that inserts multimedia");
    $ewrapper->field_references->set(array('value' => $item['references']));
  
  
    //Term reference and taxonomy fields
    $authors_vocab = taxonomy_vocabulary_machine_name_load('authors');
    $vid = $authors_vocab->vid;
    $authors_string = $item['authors'];
    $authors_array = explode('; ', $authors_string);
    $tid_array = array();
    foreach($authors_array as $author) {
        $tid = custom_create_taxonomy_term($author, $vid);
        $tid_array[] = $tid;
    }
    $ewrapper->field_article_authors->set($tid_array);
  
    $topics_vocab = taxonomy_vocabulary_machine_name_load('topics');
    $vid = $topics_vocab->vid;
    $topics_string = $item['topics'];
    $topics_array = explode(', ', $topics_string);
    $tid_array = array();
    foreach($topics_array as $topic) {
        $tid = custom_create_taxonomy_term($topic, $vid);
        $tid_array[] = $tid;
    }
    $ewrapper->field_article_topics->set($tid_array);
    
    $sections_vocab = taxonomy_vocabulary_machine_name_load('sections');
    $vid = $sections_vocab->vid;
    $sections_string = $item['sections'];
    $sections_array = explode(', ', $sections_string);
    $tid_array = array();
    foreach($sections_array as $section) {
        $tid = custom_create_taxonomy_term($section, $vid);
        $tid_array[] = $tid;
    }
    $ewrapper->field_article_sections->set($tid_array);
    
    
    
    
    
    // Now just save the wrapper and the entity
    $ewrapper->save();
  

      // Set message.
      drupal_set_message(t('Updated %title', array('%title' => $node->title)));
      $source->log('import', 'Updated %title', array('%title' => $node->title), WATCHDOG_INFO);
  }

  function custom_create_taxonomy_term($name, $vid) {

    $term_name = taxonomy_get_term_by_name($name);
    //var_dump($term_name);
    if (is_array($term_name)) {
      $term_name = array_values($term_name)[0];
      if (isset($term_name->name)) {
          //if there is a term, return the id of the term
        return $term_name->tid;
      }else{
          //if there is no term it creates the term and returns the id
        $term = new stdClass();
        $term->name = $name;
        $term->vid = $vid;
        taxonomy_term_save($term);
        return $term->tid;
      }
    }
    else {
      //if there is no term it creates the term and returns the id
      $term = new stdClass();
      $term->name = $name;
      $term->vid = $vid;
      taxonomy_term_save($term);
      return $term->tid;
    }
  }

  function contains_substr($mainStr, $str, $loc = false) {
    if ($loc === false) return (strpos($mainStr, $str) !== false);
    if (strlen($mainStr) < strlen($str)) return false;
    if (($loc + strlen($str)) > strlen($mainStr)) return false;
    return (strcmp(substr($mainStr, $loc, strlen($str)), $str) == 0);
}

}
